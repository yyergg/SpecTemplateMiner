#include<vector>
#include<iostream>
#include<fstream>
#include<vector>
#include<map>
#include<string>
#include<sstream>
using namespace::std;
#include "miner.h"

vector<vector<AndroidEvent*> > traceSet;
map<string,int> allStateEvents;
map<string,int> allViewEvents;
double confidenceThreshold=0.5;
double supportThreshold=0.7;

void miningTemplate_01(RuleNode* initRule, vector<Label*> &previousLabels){
	//cout<<"miningTemplate_01"<<endl;
	int i,j,k;
	//find all the next event in the trace set and count the appear times
	map<string,int> allNextEvents;
	for(i=0;i<previousLabels.size();i++){
		if(previousLabels[i]->eventNum < traceSet[previousLabels[i]->traceNum].size()-2){
<<<<<<< HEAD
			string combinedName=traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+1]->name; 
=======
			string combinedName=traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+1]->name;  
>>>>>>> 9a210ba52642d3825bc68f6b952ed590586847ce
			if(allNextEvents.count(combinedName)>0){
				allNextEvents[combinedName]++;
			} 
			else{
				allNextEvents[combinedName]=1;
			}
		}
	}
	map<string,int>::iterator it;
	for(it=allNextEvents.begin();it!=allNextEvents.end();it++){
		RuleNode* newViewRuleNode=new RuleNode;
		newViewRuleNode->name=it->first;
		int counter=0;
		vector<Label*> nextLabels;
		for(i=0;i<previousLabels.size();i++){
			if(previousLabels[i]->eventNum < traceSet[previousLabels[i]->traceNum].size()-2){		
				if(traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+1]->name==it->first){
					counter=counter+1;
					Label* nextLabel=new Label(previousLabels[i]->traceNum,previousLabels[i]->eventNum+1);
					nextLabels.push_back(nextLabel);	
				}
			}
		}
		map<string,int> allNextNextEvents;
		for(i=0;i<nextLabels.size();i++){
			string combinedName=traceSet[nextLabels[i]->traceNum][nextLabels[i]->eventNum+1]->name;  
			if(allNextNextEvents.count(combinedName)>0){
				allNextNextEvents[combinedName]++;
			} 
			else{
				allNextNextEvents[combinedName]=1;
			}
		}
		map<string,int>::iterator it2;
		for(it2=allNextNextEvents.begin();it2!=allNextNextEvents.end();it2++){
			double a=it2->second;
			double b=it->second;
			if(a/b>=confidenceThreshold){
				vector<Label*> nextNextLabels;
				for(i=0;i<nextLabels.size();i++){
					if(traceSet[nextLabels[i]->traceNum][nextLabels[i]->eventNum+1]->name==it2->first){
						Label* nextNextLabel=new Label(nextLabels[i]->traceNum,nextLabels[i]->eventNum+1);
						nextNextLabels.push_back(nextNextLabel);
					}
				}
				RuleNode* newStateRuleNode=new RuleNode;
				newStateRuleNode->name=it2->first;
				newViewRuleNode->children.push_back(newStateRuleNode);
				miningTemplate_01(newStateRuleNode,nextNextLabels);
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


void miningTemplate_02(RuleNode* initRule, vector<Label*> &previousLabels){
	cout<<"miningTemplate_02 "<<initRule->name<<" "<<previousLabels.size()<<endl;
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
		//fill the table	
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
		for(i=0;i<currentLabels.size();i++){
			for(j=0;j<possibleStateTable[i].size();j++){
				cout<<possibleStateTable[i][j]<<" ";
			}
			cout<<endl;
		}
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
				miningTemplate_02(newStateRuleNode, nextLabels);
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


void miningTemplate_03(RuleNode* initRule, vector<Label*> &previousLabels){
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
			cout<<currentLabels.size()<<endl;
			//init the table
			vector< vector<int> > possibleStateTable;
			possibleStateTable.resize(currentLabels.size());
			for(i=0;i<currentLabels.size();i++){ 
				possibleStateTable[i].resize(allStateEvents.size());
				for(j=0;j<allStateEvents.size();j++){
					possibleStateTable[i][j]=0;
				}
			}
			//fill the table	
			for(i=0;i<currentLabels.size();i++){
				for(j=currentLabels[i]->eventNum+1;j<traceSet[currentLabels[i]->traceNum].size();j++){
					for(it2=allStateEvents.begin();it2!=allStateEvents.end();it2++){
						if(traceSet[currentLabels[i]->traceNum][j]->name==it2->first){
							possibleStateTable[i][distance(allStateEvents.begin(),it2)]=1;
						}
<<<<<<< HEAD
					}
				}
			}
			//print the table
			for(i=0;i<currentLabels.size();i++){
				for(j=0;j<possibleStateTable[i].size();j++){
					cout<<possibleStateTable[i][j]<<" ";
				}
				cout<<endl;
			}
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
=======
>>>>>>> 9a210ba52642d3825bc68f6b952ed590586847ce
					}
					RuleNode* newStateRuleNode=new RuleNode;
					newStateRuleNode->name=traceSet[nextLabels[0]->traceNum][nextLabels[0]->eventNum]->name;				
					miningTemplate_03(newStateRuleNode, nextLabels);
					newViewRuleNode->children.push_back(newStateRuleNode);			
				}
			}
<<<<<<< HEAD
=======
			//print the table
			for(i=0;i<currentLabels.size();i++){
				for(j=0;j<possibleStateTable[i].size();j++){
					cout<<possibleStateTable[i][j]<<" ";
				}
				cout<<endl;
			}
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
					miningTemplate_03(newStateRuleNode, nextLabels);
					newViewRuleNode->children.push_back(newStateRuleNode);			
				}
			}
>>>>>>> 9a210ba52642d3825bc68f6b952ed590586847ce
			if(newViewRuleNode->children.size()>0){
				initRule->children.push_back(newViewRuleNode);
			}
			else{
				delete newViewRuleNode;
			}
		}
	}	
}
<<<<<<< HEAD

void miningTemplate_04(RuleNode* initRule, vector<Label*> &previousLabels){
	cout<<"miningTemplate_04"<<endl;
	int i;
	for(i=0;i<previousLabels.size();i++){
		if(previousLabels[i]->eventNum < traceSet[previousLabels[i]->traceNum].size()-2){
			string StateName=traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+2]->name;
			if(traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+2]->type!=0 && traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+2]->type!=1){
			cout<<"~State_"<<traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum]->name<<" U "<<"(State_"<<traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+2]->name<<" && onTouchView_"<<traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+1]->name<<")"<<endl;
			}
		}
	}

}
=======
	
>>>>>>> 9a210ba52642d3825bc68f6b952ed590586847ce
	
void setupAllStateEventsAndAllViewEvents()
{
	// cout<<traceSet.size()<<endl;
	int i,j;
	for(i=0;i<traceSet.size();i++)
	{
		for(j=0;j<traceSet[i].size();j++)
		{
		    //cout<<traceSet[i][j]->type<<traceSet[i][j]->name<<endl;
			//cout<<(allStateEvents.find(traceSet[i][j]->name)!=allStateEvents.end())<<endl;
			if(traceSet[i][j]->type==STATE_NODE)
			{
				allStateEvents[traceSet[i][j]->name]=0;  // still useless so we set it = 0
			}
			else if(traceSet[i][j]->type==VIEW_NODE)  
			{
				allViewEvents[traceSet[i][j]->name]=0;   // still useless so we set it = 0
			}
			else
			{
				//somethingwrong
			}
		}
	}
}

void printRuleTree(RuleNode* root, int level){
	int i;
	for(i=0;i<level;i++){
		cout<<"           ";
	}
	cout<<root->name<<endl;
	for(i=0;i<root->children.size();i++){
		printRuleTree(root->children[i],level+1);
	}
}

void readInTraceSet(){
<<<<<<< HEAD
	//cout<<"trace01: ABABCDEBG"<<endl;
	//cout<<"trace02: ABCDC"<<endl;
	//cout<<"trace03: ABCDE"<<endl;
	//cout<<"trace04: ABCDADE"<<endl<<endl;
=======
	//ABABCDE
	//ABCDC
	//ABCDE
	//ABCDADE
>>>>>>> 9a210ba52642d3825bc68f6b952ed590586847ce
	
	AndroidEvent* newEventA=new AndroidEvent;
	AndroidEvent* newEventB=new AndroidEvent;
	AndroidEvent* newEventC=new AndroidEvent;
	AndroidEvent* newEventD=new AndroidEvent;
	AndroidEvent* newEventE=new AndroidEvent;
	AndroidEvent* newEventG=new AndroidEvent;
	newEventA->type=0; //State A
	newEventA->name="A";
	newEventB->type=1; //View B
	newEventB->name="B";
	newEventC->type=0; //State C
	newEventC->name="C";
	newEventD->type=1; //View D
	newEventD->name="D";
	newEventE->type=0; //State E
	newEventE->name="E";
	newEventG->name="G";
	
	vector<AndroidEvent*> newTrace; //trace 1 ABABCDEBG Start
	newTrace.push_back(newEventA); //State A
	newTrace.push_back(newEventB); //View  B
	newTrace.push_back(newEventA); //State A
	newTrace.push_back(newEventB); //View  B
	newTrace.push_back(newEventC); //State C
	newTrace.push_back(newEventD); //View  D
	newTrace.push_back(newEventE); //State E
	newTrace.push_back(newEventB); //View  B
	newTrace.push_back(newEventG); //State G
	traceSet.push_back(newTrace);  //trace 1 ABABCDE done
	
	vector<AndroidEvent*> newTrace01;
	newTrace01.push_back(newEventA); //State A
	newTrace01.push_back(newEventB); //View B
	newTrace01.push_back(newEventC); //State C
	newTrace01.push_back(newEventD); //View D
	newTrace01.push_back(newEventC); //State C
	traceSet.push_back(newTrace01);//trace 2 ABCDC done 
	
	vector<AndroidEvent*> newTrace02;
	newTrace02.push_back(newEventA); //State A
	newTrace02.push_back(newEventB); //View  B
	newTrace02.push_back(newEventC); //State C
	newTrace02.push_back(newEventD); //View  D	
	newTrace02.push_back(newEventE); //State E
	traceSet.push_back(newTrace02); //trace 3 ABCDE done
	
	vector<AndroidEvent*> newTrace03;
	newTrace03.push_back(newEventA); //State A
	newTrace03.push_back(newEventB); //View  B
	newTrace03.push_back(newEventC); //State C
	newTrace03.push_back(newEventD); //View  D	
	newTrace03.push_back(newEventA); //State A
	newTrace03.push_back(newEventD); //View  D
	newTrace03.push_back(newEventE); //State E
	traceSet.push_back(newTrace03); //trace 4 ABCDADE done
	//todo
	//read in trace set by parser
}

int main(int argc,char** argv){
	int i,j,k;	
	readInTraceSet();
	setupAllStateEventsAndAllViewEvents();
	RuleNode* initRuleNode=new RuleNode;
	initRuleNode->name="init01";
	map<string,int>::iterator it;
	// template01
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
		if(newRuleNode->children.size()>0){
			initRuleNode->children.push_back(newRuleNode);
		}			
	}
	printRuleTree(initRuleNode, 0);
	
	// template02
	RuleNode* initRuleNode02=new RuleNode;
	initRuleNode02->name="init02";
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
		miningTemplate_02(newRuleNode,currentLabel);
		if(newRuleNode->children.size()>0){
			initRuleNode02->children.push_back(newRuleNode);
		}
		else{
			delete newRuleNode;
		}
	}
	printRuleTree(initRuleNode02, 0);

	// template03
	RuleNode* initRuleNode03=new RuleNode;
	initRuleNode03->name="init03";
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
			RuleNode* newRuleNode=new RuleNode;
			newRuleNode->name=it->first;		
			miningTemplate_03(newRuleNode,currentLabel);
			if(newRuleNode->children.size()>0){
				initRuleNode03->children.push_back(newRuleNode);
			}
			else{
				delete newRuleNode;
			}			
		}		
	}
	printRuleTree(initRuleNode03, 0);
<<<<<<< HEAD

	// template04
=======
	/*
		// template04
>>>>>>> 9a210ba52642d3825bc68f6b952ed590586847ce
	RuleNode* initRuleNode04=new RuleNode;
	initRuleNode04->name="init04";
	for(it=allStateEvents.begin();it!=allStateEvents.end();it++)
	{
		//init labels
		vector<Label*> currentLabel;
		for(j=0;j<traceSet.size();j++)
		{
			for(k=0;k<traceSet[j].size();k++)
			{
				if(traceSet[j][k]->name==it->first)
				{
					Label* newLabel=new Label(j,k);
					currentLabel.push_back(newLabel);
				}
			}
		}
		//start mining
		RuleNode* newRuleNode=new RuleNode;
		newRuleNode->name=it->first;		
		miningTemplate_04(newRuleNode,currentLabel);
		if(newRuleNode->children.size()>0){
			initRuleNode04->children.push_back(newRuleNode);		
		}
	}
	
	/*
		// template05
	RuleNode* initRuleNode05=new RuleNode;
	initRuleNode05->name="init05";
	for(it=allStateEvents.begin();it!=allStateEvents.end();it++)
	{
		//init labels
		vector<Label*> currentLabel;
		for(j=0;j<traceSet.size();j++)
		{
			for(k=0;k<traceSet[j].size();k++)
			{
				if(traceSet[j][k]->name==it->first)
				{
					Label* newLabel=new Label(j,k);
					currentLabel.push_back(newLabel);
				}
			}
		}
		//start mining
		RuleNode* newRuleNode05=new RuleNode;
		newRuleNode05->name=it->first;		
		miningTemplate_05(newRuleNode05,currentLabel);
		initRuleNode05->children.push_back(newRuleNode05);		
	}   
	*/
	system("pause");
	return 0;
}