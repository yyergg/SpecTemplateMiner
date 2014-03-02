#define STATE_NODE 0
#define VIEW_NODE 1

class AndroidEvent{
	public:
	int type; //state or view
	string name;
	bool isTail;
	string description; // something used to connect back to real android event(useless now)
};
<<<<<<< HEAD
 
=======

>>>>>>> 17e2eb1844f40a5b37312a3d80189ad67c2f2501
class RuleNode{
	public:
	string name;
	vector<RuleNode*> children;
};

class Label{
	public:
	int traceNum;
	int eventNum;
	Label(int tNum, int eNum){
		traceNum=tNum;
		eventNum=eNum;
	}
};
<<<<<<< HEAD
=======

>>>>>>> 17e2eb1844f40a5b37312a3d80189ad67c2f2501
