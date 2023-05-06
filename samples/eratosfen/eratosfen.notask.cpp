/*$TET$$header*/

#include <iostream>
#include <templet.hpp>

const int N_PRIME = 10;
const int N_FILTER = N_PRIME - 1;

class mes : public templet::message {
public:
	mes(templet::actor* a = 0, templet::message_adaptor ma = 0) :templet::message(a, ma) {}
	int number;
};
/*$TET$*/

#pragma templet !begin(out!mes)

struct begin :public templet::actor {
	static void on_out_adapter(templet::actor* a, templet::message* m) {
		((begin*)a)->on_out(*(mes*)m);
	}

	begin(templet::engine& e) :begin() {
		begin::engines(e);
	}

	begin() :templet::actor(true),
		out(this, &on_out_adapter)
	{
		/*$TET$begin$begin*/
		counter = 2;
		/*$TET$*/
	}

	void engines(templet::engine& e) {
		templet::actor::engine(e);
		/*$TET$begin$engines*/
		/*$TET$*/
	}

	void start() {
		/*$TET$begin$start*/
		out.number = counter++;
		out.send();
		/*$TET$*/
	}

	inline void on_out(mes& m) {
		/*$TET$begin$out*/
		if (counter > 0) {
			out.number = counter++;
			out.send();
			std::cout << "the begin actor passes a number #" << m.number << std::endl;
		}

		/*$TET$*/
	}

	mes out;

	/*$TET$begin$$footer*/
	int counter;
	/*$TET$*/
};

#pragma templet filter(in?mes,out!mes)

struct filter :public templet::actor {
	static void on_in_adapter(templet::actor* a, templet::message* m) {
		((filter*)a)->on_in(*(mes*)m);
	}
	static void on_out_adapter(templet::actor* a, templet::message* m) {
		((filter*)a)->on_out(*(mes*)m);
	}

	filter(templet::engine& e) :filter() {
		filter::engines(e);
	}

	filter() :templet::actor(false),
		out(this, &on_out_adapter)
	{
		/*$TET$filter$filter*/
		prime_number = 0;
		/*$TET$*/
	}

	void engines(templet::engine& e) {
		templet::actor::engine(e);
		/*$TET$filter$engines*/
		/*$TET$*/
	}

	inline void on_in(mes& m) {
		/*$TET$filter$in*/
		_in = &m;
		do_filtering();
		/*$TET$*/
	}

	inline void on_out(mes& m) {
		/*$TET$filter$out*/
		do_filtering();
		/*$TET$*/
	}

	void in(mes& m) { m.bind(this, &on_in_adapter); }
	mes out;

	/*$TET$filter$$footer*/
	void do_filtering() {
		if (access(_in) && access(out)) {
			if (!prime_number) {
				prime_number = _in->number;
				_in->send();
				std::cout << "the filter actor #"
					<< filter_ID << " remember a number #" << prime_number << std::endl;
			}
			else {
				if (_in->number % prime_number == 0)
					_in->send();
				else {
					out.number = _in->number;
					_in->send();
					std::cout << "the filter actor #"
						<< filter_ID << " takes a number #" << prime_number << std::endl;
					out.send();
					std::cout << "the filter actor #"
						<< filter_ID << " passed a number #" << prime_number << std::endl;
				}
			}
		}
	}
	int filter_ID;
	int prime_number;
	mes* _in;;
	/*$TET$*/
};

#pragma templet stopper(in?mes)

struct stopper :public templet::actor {
	static void on_in_adapter(templet::actor* a, templet::message* m) {
		((stopper*)a)->on_in(*(mes*)m);
	}

	stopper(templet::engine& e) :stopper() {
		stopper::engines(e);
	}

	stopper() :templet::actor(false)
	{
		/*$TET$stopper$stopper*/
		/*$TET$*/
	}

	void engines(templet::engine& e) {
		templet::actor::engine(e);
		/*$TET$stopper$engines*/
		/*$TET$*/
	}

	inline void on_in(mes& m) {
		/*$TET$stopper$p*/
		prime_number = m.number;
		if (m.number == prime_number) stop(); else m.send();
		/*$TET$*/
	}

	void in(mes& m) { m.bind(this, &on_in_adapter); }

	/*$TET$stopper$$footer*/
	int prime_number;
	/*$TET$*/
};

int main()
{
	templet::engine eng;
	begin  a_begin(eng);
	stopper a_stopper(eng);

	filter* f[N_FILTER];
	for (int i = 0; i < N_FILTER; i++) f[i] = new filter(eng);

	for (int i = 0; i < N_FILTER - 1; i++)f[i + 1]->in(f[i]->out);

	f[0]->in(a_begin.out);
	a_stopper.in(f[N_FILTER - 1]->out);

	eng.start();

	for (int i = 0; i < N_FILTER; i++)
		std::cout << f[i]->prime_number << "  ";
	std::cout << a_stopper.prime_number << "\nTotal prime numbers: #" << N_PRIME << std::endl;
}
/*$TET$*/