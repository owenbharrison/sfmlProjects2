struct expr {
	virtual float eval()=0;
};

struct numb : expr {
	float val=0;
};

struct oper : expr {
	expr* a=nullptr, * b=nullptr;
};

struct mult : oper {
	float eval() {
		return a->eval()*b->eval();
	}
};

struct divd : oper {
	float eval() {
		return a->eval()/b->eval();
	}
};