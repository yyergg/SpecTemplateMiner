#include<vector>
#include<iostream>
#include<fstream>
#include<vector>
#include<map>
#include<string>
using namespace::std;
#include "miner.h"

aaa
vector<vector<AndroidEvent*> > traceSet;
map<string,int> allStateEvents;
map<string,int> allViewEvents;
double confidenceThreshold=0.5;

void miningTemplate_01(RuleNode* initRule, vector<Label*> &previousLabels)
{
	cout<<"miningTemplate_01"<<endl;
	int i,j,k;
	//find all the next event in the trace set and count the appear times
	map<string,int> allNextEvents;
	for(i=0;i<previousLabels.size();i++)
	{
		if(previousLabels[i]->eventNum < traceSet[previousLabels[i]->traceNum].size()-2) 	// traceset[A->traceNum].size() is the tail of the trace which includes A
		{
			string combinedName=traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+1]->name  //+1:event 1
			+"+"+traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+2]->name; // +2:state 2
			if(allNextEvents.count(combinedName)>0){
				allNextEvents[combinedName]++;
			} 
			else{
				allNextEvents[combinedName]=1;
			}
			cout <<"after: "<<combinedName<<" "<<allNextEvents[combinedName]<<endl;
		}
	}
	//calculate confidence and recursive
	map<string,int>::iterator it;
	for(it=allNextEvents.begin();it!=allNextEvents.end();it++)
	{
		double a=it->second;
		double b=previousLabels.size();
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
			for(i=0;i<previousLabels.size();i++)
			{
				if(previousLabels[i]->eventNum < traceSet[previousLabels[i]->traceNum].size()-2)
				{
					if(traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+1]->name==childViewEventName && 
					traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+2]->name==childStateEventName)
					{
						Label* newLabel=new Label(previousLabels[i]->traceNum,previousLabels[i]->eventNum+2);
						nextLabels.push_back(newLabel);
					}					
				}
			}
			//add to children
			RuleNode* newViewRuleNode=new RuleNode;
			newViewRuleNode->name=childViewEventName;
			RuleNode* newStateRuleNode=new RuleNode;
			newStateRuleNode->name=childStateEventName;
			initRule->children.push_back(newViewRuleNode);
			newViewRuleNode->children.push_back(newStateRuleNode);
			//recursive
			miningTemplate_01(newStateRuleNode,nextLabels);
		}
	}
}

/*********** Conditional Eventually *************/ 
/************************************************/ 
void miningTemplate_02(RuleNode* initRule, vector<Label*> &previousLabels) 
{
	cout<<"miningTemplate_02"<<endl;
	int i,j,k;
	map<string,int>::iterator it;
	map<string,int>::iterator it2;
	map<string,int> possibleViews;
	vector<Label*> possibleViewsLabel;
	for(i=0;i<previousLabels.size();i++)
	{
		if(previousLabels[i]->eventNum < traceSet[previousLabels[i]->traceNum].size()-2)
		{
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
				cout<<it2->first<<endl;
				map<string,int> labelHash;
				for(i=0;i<currentLabels.size();i++){
					if(possibleStateTable[i][distance(allStateEvents.begin(),it2)]==1){
						while(traceSet[currentLabels[i]->traceNum][currentLabels[i]->eventNum]->name!=it2->first){							
							currentLabels[i]->eventNum=currentLabels[i]->eventNum+1;
						}
						string key=currentLabels[i]->traceNum+"+"+currentLabels[i]->eventNum;
						if(labelHash.count(key)==0){
							labelHash[key]=1;
						}
						else{
							cout<<"should remove"<<endl;
							//currentLabels.erase(currentLabels.begin()+i);
							//i--;
						}
					}
					else{
						cout<<"should remove"<<endl;
						//currentLabels.erase(currentLabels.begin()+i);
						//i--;					
					}
				}
				RuleNode* newStateRuleNode=new RuleNode;
				newStateRuleNode->name=traceSet[currentLabels[0]->traceNum][currentLabels[0]->eventNum]->name;				
				//miningTemplate_02(newStateRuleNode, currentLabels);
				newViewRuleNode->children.push_back(newStateRuleNode);			
			}
		}
		if(newViewRuleNode->children.size()>0){
			initRule->children.push_back(newViewRuleNode);
		}
		cout<<endl;
	}	
/*	
	vector< vector<int> > table;
	table.resize(possibleViews.size());
	for(i=0;i<table.size();i++){ 
		table[i].resize(allStateEvents.size());
		for(j=0;j<allStateEvents.size();j++){
			table[i][j]=0;
		}
	}

	for(i=0;i<possibleViews.size();i++){
		for(j=0;j<allStateEvents.size();j++){
			for(k=0;k<traceSet[possibleViews[i]->traceNum].size();k++){
				if(traceSet[possibleViews[i]->traceNum][k]->name==possibleViews[i]->name){
					table[j][k]=1;
				}
			}
		}
	}*/
/*
	//calculate the number of StateEvent
	int count=0;
	for(i=0;i<possibleViews.size();i++)	
	{

		for(j=0;j<allStateEvents.size();j++)
		{
			if(table[i][j]=1)
			{
				count++;
			}
		}
	}
	//calculate confidence and recursive
	map<string,int>::iterator it;
	for(it=allNextEvents.begin();it!=allNextEvents.end();++it)
	{
		double a=it->second;
		double b=previousLabels.size();
		if( a/b >= confidenceThreshold)
		{
			//seperate the combinedName
			for(i=0;i < it->first.size();i++)
			{
				if(it->first[i]=='+')
				{
					break;
				}
			}
			string childViewEventName=it->first.substr(0,i);
			string childStateEventName=it->first.substr(i+1,it->first.size()-i-1);
			//update labels
			vector<Label*> nextLabels;
			for(i=0;i<previousLabels.size();i++)
			{
				if(previousLabels[i]->eventNum < traceSet[previousLabels[i]->traceNum].size()-2)
				{
					if(traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+1]->name==childViewEventName && 
					traceSet[previousLabels[i]->traceNum][previousLabels[i]->eventNum+2]->name==childStateEventName)
					{
						Label* newLabel=new Label(previousLabels[i]->traceNum,previousLabels[i]->eventNum+2);
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
			miningTemplate_02(newStateRuleNode,nextLabels);
		}
	}	*/
}


/*********** Eventually *************/ 
/************************************/ 
/*
void miningTemplate_03(RuleNode* initRule, vector<Label*> &previousLabels) 
{
	int i,j,k;
	map<string,int>::iterator it;
	map<string,int> possibleViews;
	map<string,int> allNextEvents;
	for(i=0;i<previousLabels.size();i++)
	{
		vector <vector<int>> table;
		table.resize(possibleViews.size());
		for(i=0;i<table.size();i++)
		{ 
			table[i].resize(allStateEvents.size());
			for(j=0;j<allStateEvents.size();j++)
			{
				table[i][j]=0;
			}
		}
 
		for(it=possibleViews.begin();it!=possibleViews.end();it++)			
		{
			for(j=0;j<allStateEvents.size();j++)
			{
				for(k=0;k<traceSet[j].size();k++)
				{
					if(traceSet[j][k]->name==it1->first)
					{
						table[j][k]=1;
					}
				}
			}
		}
	}
}
	
	//  ==============================================================*/


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

void readInTraceSet()
{
	AndroidEvent* newEventA=new AndroidEvent;
	AndroidEvent* newEventB=new AndroidEvent;
	AndroidEvent* newEventC=new AndroidEvent;
	AndroidEvent* newEventD=new AndroidEvent;
	AndroidEvent* newEventE=new AndroidEvent;
	
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
	
	vector<AndroidEvent*> newTrace; //trace 1 ABABCDE Start
	newTrace.push_back(newEventA); //State A
	newTrace.push_back(newEventB); //View  B
	newTrace.push_back(newEventA); //State A
	newTrace.push_back(newEventB); //View  B
	newTrace.push_back(newEventC); //State C
	newTrace.push_back(newEventD); //View  D
	newTrace.push_back(newEventE); //State E
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

int main(int argc,char** argv)
{
	int i,j,k;	
	readInTraceSet();
	setupAllStateEventsAndAllViewEvents();
	RuleNode* initRuleNode=new RuleNode;
	initRuleNode->name="init01";
	map<string,int>::iterator it;
	// template01
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
		miningTemplate_01(newRuleNode,currentLabel);
		if(newRuleNode->children.size()>0){
			initRuleNode->children.push_back(newRuleNode);
		}			
	}
	printRuleTree(initRuleNode, 0);
	
	// template02
	RuleNode* initRuleNode02=new RuleNode;
	initRuleNode02->name="init02";
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
		RuleNode* newRuleNode02=new RuleNode;
		newRuleNode02->name=it->first;		
		miningTemplate_02(newRuleNode02,currentLabel);
		initRuleNode02->children.push_back(newRuleNode02);		
	}
/*
	// template03
	RuleNode* initRuleNode03=new RuleNode;
	initRuleNode03->name="init03";
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
		RuleNode* newRuleNode03=new RuleNode;
		newRuleNode03->name=it->first;		
		miningTemplate_03(newRuleNode03,currentLabel);
		initRuleNode03->children.push_back(newRuleNode03);		
	}
	
		// template04
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
		RuleNode* newRuleNode03=new RuleNode;
		newRuleNode04->name=it->first;		
		miningTemplate_04(newRuleNode04,currentLabel);
		initRuleNode04->children.push_back(newRuleNode04);		
	}
	
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
	
	return 0;
}