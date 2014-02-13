#include<vector>
#include<iostream>
#include<fstream>
#include<vector>
#include<map>
#include<string>
using namespace::std;
#include "miner.h"


vector<vector<AndroidEvent*> > traceSet;
map<string,int> allStateEvents;
map<string,int> allViewEvents;
double confidenceThreshold=0.5;

void miningTemplate_01(RuleNode* initRule, vector<Label*> &previousLables){
	int i,j,k;
	//find all the next event in the trace set and count the appear times
	map<string,int> allNextEvents;
	for(i=0;i<previousLables.size();i++){
		if(previousLables[i]->eventNum < traceSet[previousLables[i]->traceNum].size()-2){
			//combine the name of the 2 following event as hashkey
			string combinedName=traceSet[previousLables[i]->traceNum][previousLables[i]->eventNum+1]->name
			+"+"+traceSet[previousLables[i]->traceNum][previousLables[i]->eventNum+2]->name;
			if(allNextEvents.find(combinedName)!=allNextEvents.end()){
				allNextEvents[combinedName]++;
			}
			else{
				allNextEvents[combinedName]=1;
			}
		}
	}
	//calculate confidence and recursive
	map<string,int>::iterator it;
	for(it=allNextEvents.begin();it!=allNextEvents.end();++it){
		double a=it->second;
		double b=previousLables.size();
		if( a/b >= confidenceThreshold){
			//seperate the combinedName
			for(i=0;i < it->first.size();i++){
				if(it->first[i]=='+'){
					break;
				}
			}
			string childViewEventName=it->first.substr(0,i);
			string childStateEventName=it->first.substr(i+1,it->first.size()-i-1);
			//update labels
			vector<Label*> nextLabels;
			for(i=0;i<previousLables.size();i++){
				if(previousLables[i]->eventNum < traceSet[previousLables[i]->traceNum].size()-2){
					if(traceSet[previousLables[i]->traceNum][previousLables[i]->eventNum+1]->name==childViewEventName && 
					traceSet[previousLables[i]->traceNum][previousLables[i]->eventNum+2]->name==childStateEventName){
						Label* newLabel=new Label(previousLables[i]->traceNum,previousLables[i]->eventNum+2);
						nextLabels.push_back(newLabel);
					}					
				}
			}
			//add to children
			RuleNode* newStateRuleNode=new RuleNode;
			newStateRuleNode->name=childStateEventName;
			RuleNode* newViewRuleNode=new RuleNode;
			newViewRuleNode->name=childViewEventName;
			initRule->children.push_back(newViewRuleNode);
			newViewRuleNode->children.push_back(newStateRuleNode);
			//recursive
			miningTemplate_01(newStateRuleNode,nextLabels);
		}
	}
}

void setupAllStateEventsAndAllViewEvents(){
	int i,j;
	for(i=0;i<traceSet.size();i++){
		for(j=0;j<traceSet[i].size();j++){
			if(traceSet[i][j]->type==STATE_NODE && allStateEvents.find(traceSet[i][j]->name)!=allStateEvents.end()){
				allStateEvents[traceSet[i][j]->name]=0;
			}
			else if(traceSet[i][j]->type==VIEW_NODE && allViewEvents.find(traceSet[i][j]->name)!=allViewEvents.end()){
				allStateEvents[traceSet[i][j]->name]=0;
			}
			else
			{
				//somethingwrong
			}
		}
	}
}

void readInTraceSet()
{
	//todo
	//read in trace set by parser
}

int main(int argc,char** argv){
	int i,j,k;	
	readInTraceSet();
	setupAllStateEventsAndAllViewEvents();
	RuleNode* initRuleNode=new RuleNode;
	initRuleNode->name="init";
	map<string,int>::iterator it;
	for(it=allStateEvents.begin();it!=allStateEvents.end();it++){
		//init labels
		vector<Label*> currentLabel;
		for(j=0;j<traceSet.size();j++){
			for(k=0;k<traceSet[j].size();k++){
				if(traceSet[j][k]->name==it->first){
					Label* newLabel=new Label(j,k);
					currentLabel.push_back(newLabel);
				}
			}
		}
		//start mining
		RuleNode* newRuleNode=new RuleNode;
		newRuleNode->name=it->first;		
		miningTemplate_01(newRuleNode,currentLabel);
		initRuleNode->children.push_back(newRuleNode);		
	}
	return 0;
}