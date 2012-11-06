///////////////////////////////////////////////////////////////////////////////
// Type variadic vector.
// Mathias Rav, Jungwoo Yang, November 2, 2012.
///////////////////////////////////////////////////////////////////////////////

#include <iostream> // for my_visitor, using std::cout
#include <vector> // type non-variadic vector basis
#include <stdexcept>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <typeinfo>

// Variadic type vector visitor concept:
// Must have a operator() that is templated to accept
// any of the types used in the vector
class my_visitor {
public:
	typedef void result_type;

	template <typename T>
	void operator()(T & v) {
		std::cout << typeid(T).name() << ' ' << v << std::endl;
	}
};

// The following complexity exploits the C++ language principle named
// SFINAE (Substitution Failure Is Not An Error)

// A "type list" is either void (the empty list of types)
// or a type T and a type list Ts in std::pair<T, Ts>

// This class is used to test if one type wins against another in an overload
// resolution setting.
template <typename Candidate, typename Opponent>
struct better_match_helper {
	static char foo(Candidate);
	static short foo(Opponent);
	static short foo(...);
};

template <size_t sz>
struct onetwo_helper;

template <>
struct onetwo_helper<sizeof(char)> : public boost::true_type {};

template <>
struct onetwo_helper<sizeof(short)> : public boost::false_type {};

// true_type if Candidate is a better choice than Opponent when given parameter
// of type Check;
// false_type otherwise
template <typename Check, typename Candidate, typename Opponent>
struct better_match : public onetwo_helper<sizeof(better_match_helper<Candidate, Opponent>::foo(*((Check*)0)))>
{};

// true_type if one of the types in Types is a better match than Candidate
// given Check
template <typename Candidate, typename Types, typename Check>
struct exists_better_match;

// In case of the empty list, there is not a better match
template <typename Candidate, typename Check>
struct exists_better_match<Candidate, void, Check> {
	static const bool value = false;
};

// Test first element of list, or recurse into the rest of the type list
template <typename Candidate, typename T, typename Next, typename Check>
struct exists_better_match<Candidate, std::pair<T, Next>, Check> {
	static const bool value =
		(boost::is_convertible<Check, T>::value
		&& better_match<Check, T, Candidate>::value)
		|| (exists_better_match<Candidate, Next, Check>::value);
};

template <typename Current, typename Next, typename Check>
struct check_closest_match {
	static const bool value =
		boost::is_convertible<Check, Current>::value
		&& !exists_better_match<Current, Next, Check>::value;
};

// Alias of the above
template <typename Current, typename Next, typename Check>
struct closest_match
: public boost::enable_if<check_closest_match<Current, Next, Check> >
{
};

// Alias of the above
template <typename Current, typename Next, typename Check>
struct not_closest_match
: public boost::disable_if<check_closest_match<Current, Next, Check> >
{
};

// `Types` is either the empty type list `void`
// or a type and a type list `std::pair<T, Ts>`
template <typename Types>
class varvector;

// Base case: Empty type list
template <>
class varvector<void> {
protected:
	typedef size_t it_initializer;
	size_t m_size;

	it_initializer make_begin() {
		return 0;
	}

	it_initializer make_end() {
		return m_size;
	}
public:
	varvector() : m_size(0) { }
	size_t size() const {
		return m_size;
	}

	class iterator {
		size_t i;
		size_t last;
	protected:
		size_t idx() const {
			return i;
		}
		void advance() {
			++i;
		}
	public:
		iterator(it_initializer i, it_initializer last)
			: i(i), last(last)
		{
		}

		template <typename Fn>
		typename Fn::result_type operator()(Fn &) {
			throw std::runtime_error("Iterator index out of bounds");
		}
	};
};

template <typename Current, typename Next>
class varvector<std::pair<Current, Next> > : public varvector<Next> {
	typedef varvector<Next> p_t;
	typedef std::vector<std::pair<size_t, Current> > storage;
	storage items;
protected:
	typedef typename storage::iterator it;
	typedef std::pair<it, typename p_t::it_initializer> it_initializer;

	it_initializer make_begin() {
		return std::make_pair(items.begin(), p_t::make_begin());
	}

	it_initializer make_end() {
		return std::make_pair(items.end(), p_t::make_end());
	}
public:
	class iterator : public p_t::iterator {
		typedef typename p_t::iterator pi_t;
		it i;
		it last;
	protected:
		size_t idx() const {
			return pi_t::idx();
		}
		void advance() {
			if (i != last && i->first == idx()) ++i;
			pi_t::advance();
		}
	public:
		iterator(it_initializer from, it_initializer last)
			: pi_t(from.second, last.second)
			, i(from.first)
			, last(last.first)
		{
		}
		void operator++() {
			advance();
		}
		template <typename Fn>
		typename Fn::result_type operator()(Fn & f) {
			if (i != last && i->first == idx()) return f(i->second);
			else return pi_t::operator()(f);
		}
		bool operator==(const iterator & other) const {
			return idx() == other.idx();
		}
		bool operator!=(const iterator & other) const {
			return idx() != other.idx();
		}
		bool operator<(const iterator & other) const {
			return idx() < other.idx();
		}
	};
	iterator begin() {
		return iterator(make_begin(), make_end());
	}
	iterator end() {
		return iterator(make_end(), make_end());
	}
	template <typename T>
	void push_back(T a, typename closest_match<Current, Next, T>::type * = 0) {
		items.push_back(std::make_pair(this->size(), a));
		++this->m_size;
	}
	template <typename T>
	void push_back(T a, typename not_closest_match<Current, Next, T>::type * = 0) {
		p_t::push_back(a);
	}
};

template <typename T1, typename T2, typename T3>
void test() {
	typedef varvector<std::pair<T1, std::pair<T2, std::pair<T3, void> > > > vector_type;
	typedef typename vector_type::iterator iterator;
	vector_type a;
	short s = 2;
	a.push_back(s);
	a.push_back(42);
	a.push_back("Hello world!");
	a.push_back(42.5);
	a.push_back(42);
	a.push_back(42.5);
	my_visitor v;
	for (iterator i = a.begin(); i != a.end(); ++i) {
		i(v);
	}
}

int main() {
	test<int, const char *, double>();
	test<const char *, double, int>();
	test<double, int, const char *>();
	return 0;
}
