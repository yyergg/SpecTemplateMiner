#include <vector>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
using namespace::std;
#include "miner.h"

int populationSize = 100;
int maxRuleDepth = 3;
vector<vector<AndroidEvent*> > traceSet;
map<string,int> allStateEvents;
map<string,int> allViewEvents;
double confidenceThreshold=0.7;
double supportThreshold=0.3;
vector<bool> passTable;

bool weightcmp(const Weight* w1, const Weight* w2){
    return (w1->score < w2->score);
}

void selection(vector<Weight*> &weights){
    int i,j;
    sort(weights.begin(), weights.end(), weightcmp);
    int removeSize = weights.size()*0.3;
    for(i=0;i<removeSize;i++){
        delete weights[i];
    }
    weights.erase(weights.begin(),weights.begin()+removeSize);
}

void crossover(vector<Weight*> &weights){
    int i,j;
    while(true){
        Weight* p1;
        Weight* p2;
        p1 = weights[rand()%weights.size()];
        p2 = weights[rand()%weights.size()];
        int cutpoint=rand()%(p1->weight.size());
        Weight* c1=new Weight;
        Weight* c2=new Weight;
        for(j=0;j<p1->weight.size();j++){
            if(j<cutpoint){
                c1->weight.push_back(p1->weight[j]);
                c2->weight.push_back(p2->weight[j]);
            }
            else{
                c1->weight.push_back(p2->weight[j]);
                c2->weight.push_back(p1->weight[j]);
            }
            c1->threshold = p1->threshold;
            c2->threshold = p2->threshold;           
        }
        if(weights.size() == populationSize - 1){
            weights.push_back(c1);
            break;
        }
        else if(weights.size() == populationSize - 2){
            weights.push_back(c1);
            weights.push_back(c2);
            break;
        }
        else{
            weights.push_back(c1);
            weights.push_back(c2);
        }
    }
}


void printWeights(vector<Weight*> &weights){
    int i,j;
    for(i=0;i<weights.size();i++){
        for(j=0;j<weights[i]->weight.size();j++){
            cout<<weights[i]->weight[j]<<" ";
        }
        cout<<"|"<<weights[i]->threshold;
        cout<<"|"<<weights[i]->score<<endl;
    }
}


void calculateScore(Weight* w,vector<vector<bool> > tableSat){
    int i,j;
    int sum;
    int score;
    score = 0;
    for(i=0;i<tableSat.size();i++){
        sum = 0;
        for(j=0;j<tableSat[i].size();j++){
            if(tableSat[i][j] == true){
                sum = sum + w->weight[j];
            }
        }
        if(sum > w->threshold){
            score++;
        }
    }
    w->score = score;
}

void labelTree(RuleNode* posRule){
    int i,j;
    for(i=0;i<posRule->children.size();i++){
        for(j=0;j<posRule->mappedTo->children.size();j++){
            if(posRule->children[i]->name.compare(posRule->mappedTo->children[j]->name) == 0){
                posRule->children[i]->mappedTo = posRule->mappedTo->children[j];
                posRule->mappedTo->children[j]->labeled = true;
                labelTree(posRule->children[i]);
                break;
            }
        }
    }
}

bool checkRemovable(RuleNode* negRule){
    cout<<"checking "<<negRule->name<<endl;
    int i;
    for(i=0;i<negRule->children.size();i++){
        if(checkRemovable(negRule->children[i])){
            negRule->children.erase(negRule->children.begin()+i);
            i--;
        }
    }
    if(negRule->children.size() == 0 && negRule->labeled == true){
        cout<<"remove:"<<negRule->name<<endl;
        return true;
    }
    return false;
}


void removeTree(RuleNode* posRule, RuleNode* negRule){
    posRule->mappedTo=negRule;
    labelTree(posRule);
    checkRemovable(negRule);
}



void miningTemplate_03(RuleNode* initRule, vector<Label*> &previousLabels, int currentDepth){
    cout<<"miningTemplate_03 "<<initRule->name<<" "<<previousLabels.size()<<endl;
    int i,j,k;
    map<string,int>::iterator it;
    map<string,int>::iterator it2;
    map<string,int> possibleViews;
    vector<Label*> possibleViewsLabel;
    for(i=0;i<previousLabels.size();i++){
        if(previousLabels[i]->eventNum < traceSet[previousLabels[i]->traceNum].size()-2){
            if(possibleViews.count(traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+1]->name)>0){
                possibleViews[traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+1]->name]++;
            }
            else{
                possibleViews[traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+1]->name]=1;
            }
            Label* newLabel=new Label(previousLabels[i]->traceNum,previousLabels[i]->eventNum+1);
            possibleViewsLabel.push_back(newLabel);
        }
    }

    for(it=possibleViews.begin();it!=possibleViews.end();it++){
        cout<<it->first<<" "<<it->second<<endl;
        double x=it->second;
        double y=previousLabels.size();
        if(x/y>=supportThreshold){
            RuleNode* newViewRuleNode=new RuleNode;
            newViewRuleNode->name=it->first;
            vector<Label*> currentLabels; 
            for(i=0;i<possibleViewsLabel.size();i++){
                if(traceSet[possibleViewsLabel[i]->traceNum][possibleViewsLabel[i]->eventNum]->name==it->first){
                    currentLabels.push_back(possibleViewsLabel[i]);
                }
            }
            //init the table
            vector< vector<int> > possibleStateTable;
            possibleStateTable.resize(currentLabels.size());
            for(i=0;i<currentLabels.size();i++){ 
                possibleStateTable[i].resize(allStateEvents.size());
                for(j=0;j<allStateEvents.size();j++){
                    possibleStateTable[i][j]=0;
                }
            }
            //fill in the table
            for(i=0;i<currentLabels.size();i++){
                for(j=currentLabels[i]->eventNum+1;j<traceSet[currentLabels[i]->traceNum].size();j++){
                    for(it2=allStateEvents.begin();it2!=allStateEvents.end();it2++){
                        if(traceSet[currentLabels[i]->traceNum][j]->name==it2->first){
                            possibleStateTable[i][distance(allStateEvents.begin(),it2)]=1;
                        }
                    }
                }
            }
            //print the table
            //for(i=0;i<currentLabels.size();i++){
            //    for(j=0;j<possibleStateTable[i].size();j++){
            //        cout<<possibleStateTable[i][j]<<" ";
            //    }
            //    cout<<endl;
            //}
            for(it2=allStateEvents.begin();it2!=allStateEvents.end();it2++){
                double a=0.0;
                for(i=0;i<currentLabels.size();i++){
                    if(possibleStateTable[i][distance(allStateEvents.begin(),it2)]==1){
                        a=a+1.0;
                    }
                }
                double b=currentLabels.size();
                
                if(a/b>=confidenceThreshold){
                    //setup next labels
                    vector<Label*> nextLabels;
                    map<string,int> labelHash;
                    for(i=0;i<currentLabels.size();i++){
                        Label* newNextLabel=new Label(currentLabels[i]->traceNum,currentLabels[i]->eventNum);
                        if(possibleStateTable[i][distance(allStateEvents.begin(),it2)]==1){
                            while(traceSet[newNextLabel->traceNum][newNextLabel->eventNum]->name!=it2->first){
                                newNextLabel->eventNum=newNextLabel->eventNum+1;
                            }
                            stringstream key;
                            key<<newNextLabel->traceNum<<"+"<<newNextLabel->eventNum;
                            if(labelHash.count(key.str())==0){
                                labelHash[key.str()]=1;
                                nextLabels.push_back(newNextLabel);
                            }
                            else{
                                cout<<"(repeat)should be ignored "<<endl;
                            }
                        }
                        else{
                            cout<<"(no next step)should be ignored "<<endl;
                        }
                    }
                    RuleNode* newStateRuleNode=new RuleNode;
                    newStateRuleNode->name=traceSet[nextLabels[0]->traceNum][nextLabels[0]->eventNum]->name;
                    if (currentDepth<maxRuleDepth){
                        miningTemplate_03(newStateRuleNode, nextLabels,currentDepth+1);
                    }
                    newViewRuleNode->children.push_back(newStateRuleNode);
                }
            }
            if(newViewRuleNode->children.size()>0){
                initRule->children.push_back(newViewRuleNode);
            }
            else{
                delete newViewRuleNode;
            }
        }
    }
}




void setupAllStateEventsAndAllViewEvents(){
    // cout<<traceSet.size()<<endl;
    int i,j;
    for(i=0;i<traceSet.size();i++){
        for(j=0;j<traceSet[i].size();j++){
            //cout<<traceSet[i][j]->type<<traceSet[i][j]->name<<endl;
            //cout<<(allStateEvents.find(traceSet[i][j]->name)!=allStateEvents.end())<<endl;
            if(traceSet[i][j]->type==STATE_NODE){
                allStateEvents[traceSet[i][j]->name]=0;  // still useless so we set it = 0
            }
            else if(traceSet[i][j]->type==VIEW_NODE){
                allViewEvents[traceSet[i][j]->name]=0;   // still useless so we set it = 0
            }
            else{
                //somethingwrong
            }
        }
    }
}


void treeToMatrix(RuleNode* root, vector<RuleNode*> &stack, vector< vector<RuleNode*> > &result){
    stack.push_back(root);
    if(root->children.size()==0){
        vector<RuleNode*> newRule=stack;
        result.push_back(newRule);
    }
    else{
        int i=0;
        for(i=0;i<root->children.size();i++){
            treeToMatrix(root->children[i],stack,result);
        }
    }
    stack.pop_back();
}



void printRuleTree(RuleNode* root, int level){
    int i;
    for(i=0;i<level;i++){
        cout<<"             ";
    }
    cout<<root->name<<endl;
    for(i=0;i<root->children.size();i++){
        printRuleTree(root->children[i],level+1);
    }
}


void readInTraceSet(char* inFilename){
    fstream infile;
    infile.open(inFilename, ios::in);
    string tempStr;
    infile >> tempStr;
    while (!infile.eof()){
        vector<AndroidEvent*> newTrace;
        while (tempStr != "Pass" && tempStr != "Fail"){
            AndroidEvent* newEvent = new AndroidEvent();
            std::size_t found = tempStr.find('_');
            if (found == std::string::npos){ newEvent->type = 0; }
            else{ newEvent->type = 1; }
            newEvent->name = tempStr;
            newTrace.push_back(newEvent);
            infile >> tempStr;
        }
        traceSet.push_back(newTrace);
        if (tempStr == "Pass"){
            passTable.push_back(true);
        }
        else{
            passTable.push_back(false);
        }
        infile >> tempStr;
    }
    infile.close();
}

bool ruleChecker03(vector<AndroidEvent*> trace, vector<RuleNode*>rule){
    int i,j,k;
    int lastPos=0;
    i=1;
    while (i < rule.size()-1){
        bool stepSat=false;
        cout<<"searching for "<<rule[i]->name<<" and "<<rule[i+1]->name<<endl;
        for(j=lastPos;j<trace.size()-1;j++){
            if(trace[j]->name.compare(rule[i]->name)==0 && trace[j+1]->name.compare(rule[i+1]->name)==0){
                stepSat=true;
                lastPos=j+2;
                i=i+2;
                cout<<"found at "<<j<<endl;
                break;
            }
        }
        if(!stepSat){
            return false;
        }
    }
    cout<<"searching for "<<rule[rule.size()-1]->name<<endl;
    for(i=lastPos;i<trace.size();i++){
        if(trace[i]->name.compare(rule[rule.size()-1]->name)==0){
            return true;
        }
    }
    return false;
}

int main(int argc,char** argv){
    int i,j,k;
    cout<<"Loading traces..."<<endl;
    readInTraceSet(argv[1]);
    cout<<"Loading traces finished"<<endl;
    setupAllStateEventsAndAllViewEvents();

    // template03
    RuleNode* initRuleNode03=new RuleNode;
    initRuleNode03->name="init03";
    map<string,int>::iterator it;
    for(it=allStateEvents.begin();it!=allStateEvents.end();it++){
        //init labels
        double supportCounter=0.0;
        vector<Label*> currentLabel;
        for(j=0;j<traceSet.size();j++){
            bool stateEventExist=false;
            for(k=0;k<traceSet[j].size();k++){
                if(traceSet[j][k]->name==it->first){
                    stateEventExist=true;
                    Label* newLabel=new Label(j,k);
                    currentLabel.push_back(newLabel);
                }
            }
            if(stateEventExist==true){
                supportCounter=supportCounter+1.0;
            }
        }
        double b=traceSet.size();
        //start mining
        if(supportCounter/b>=supportThreshold){
            cout<<it->first<<endl;
            RuleNode* newRuleNode=new RuleNode;
            newRuleNode->name=it->first;
            miningTemplate_03(newRuleNode,currentLabel,1);
            if(newRuleNode->children.size()>0){
                initRuleNode03->children.push_back(newRuleNode);
            }
            else{
                delete newRuleNode;
            }
        }
    }
    printRuleTree(initRuleNode03, 0);

    vector<RuleNode*> stack;
    vector< vector<RuleNode*> > result;

/*
    vector<vector<bool> > tableSat;
    for(i=0;i<traceSet.size();i++){
        vector<bool> row;
        for(j=0;j<result.size();j++){
            row.push_back(ruleChecker01(traceSet[i],result[j]));
        }
        tableSat.push_back(row);
    }
    for(i=0;i<traceSet.size();i++){
        for(j=0;j<result.size();j++){
            cout<<tableSat[i][j]<<" ";
        }
        cout<<endl;
    }    
    vector<Weight*> weights;
    for(i=0;i<populationSize;i++){
        Weight* w = new Weight;
        for(j=0;j<result.size();j++){
            w->weight.push_back(rand()%10);
        }
        w->threshold = rand()%(10*result.size());
        calculateScore(w,tableSat);
        weights.push_back(w);
    }
    printWeights(weights);



    for(i=0;i<100;i++){
        cout<<"***************************************************************"<<endl;
        selection(weights);
        crossover(weights);
        printWeights(weights);
    }
*/


    return 0;
}