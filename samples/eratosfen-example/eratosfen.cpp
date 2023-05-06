/*$TET$$header*/

#include <iostream>
#include <templet.hpp>
#include <basesim.hpp>

const int N_PRIME = 5;
const int N_FILTER = N_PRIME - 1;
const int DELAY = 1;

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

#pragma templet filter(in?mes,out!mes,t:basesim)

struct filter :public templet::actor {
	static void on_in_adapter(templet::actor* a, templet::message* m) {
		((filter*)a)->on_in(*(mes*)m);
	}
	static void on_out_adapter(templet::actor* a, templet::message* m) {
		((filter*)a)->on_out(*(mes*)m);
	}
	static void on_t_adapter(templet::actor* a, templet::task* t) {
		((filter*)a)->on_t(*(templet::basesim_task*)t);
	}

	filter(templet::engine& e, templet::basesim_engine& te_basesim) :filter() {
		filter::engines(e, te_basesim);
	}

	filter() :templet::actor(false),
		out(this, &on_out_adapter),
		t(this, &on_t_adapter)
	{
		/*$TET$filter$filter*/
		prime_number = 0;
		/*$TET$*/
	}

	void engines(templet::engine& e, templet::basesim_engine& te_basesim) {
		templet::actor::engine(e);
		t.engine(te_basesim);
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

	inline void on_t(templet::basesim_task& t) {
		/*$TET$filter$t*/
		t.delay(DELAY);
		do_filtering();
		/*$TET$*/
	}

	void in(mes& m) { m.bind(this, &on_in_adapter); }
	mes out;
	templet::basesim_task t;


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
			t.submit();
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
	templet::basesim_engine teng;

	begin  a_begin(eng);
	filter* f[N_FILTER];
	stopper a_stopper(eng);

	for (int i = 0; i < N_FILTER; i++) f[i] = new filter(eng, teng);

	for (int i = 0; i < N_FILTER - 1; i++)f[i + 1]->in(f[i]->out);

	f[0]->in(a_begin.out);
	a_stopper.in(f[N_FILTER - 1]->out);

	srand(time(NULL));

	eng.start();
	teng.run();

	a_stopper.prime_number = N_PRIME;

	for (int i = 0; i < N_FILTER; i++)
		std::cout << f[i]->prime_number << "  ";
	std::cout << "\nTotal prime numbers: #" << N_PRIME << std::endl;
	std::cout << "Maximum number of tasks executed in parallel : " << teng.Pmax() << std::endl;
	std::cout << "Time of sequential execution of all tasks    : " << teng.T1() << std::endl;
	std::cout << "Time of parallel   execution of all tasks    : " << teng.Tp() << std::endl;
}
/*$TET$*/