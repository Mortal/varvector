///////////////////////////////////////////////////////////////////////////////
// Type variadic vector.
// Mathias Rav, Jungwoo Yang, November 2, 2012.
///////////////////////////////////////////////////////////////////////////////

#include <iostream> // for my_visitor, using std::cout
#include <vector> // type non-variadic vector basis

// Variadic type vector visitor concept:
// Must have a operator() that is templated to accept
// any of the types used in the vector
class my_visitor {
public:
	template <typename T>
	void operator()(T & v) {
		std::cout << v << std::endl;
	}
};

// SFINAE (Substitution Failure Is Not An Error):
// Check if two types are the same or different.
// Equivalent to boost::enable_if<boost::is_same<T, U> >.
// #include <boost/utility.hpp>
// #include <boost/type_traits.hpp>

template <typename T, typename U>
struct same {};
template <typename T>
struct same<T, T> { typedef void type; };

// Equivalent to boost::disable_if<boost::is_same<T, U> >.
template <typename T, typename U>
struct different { typedef void type; };
template <typename T>
struct different<T, T> {};

// `Types` is either the empty type list `void`
// or a type and a type list `std::pair<T, Ts>`
template <typename Types>
class varvector;

// Base case: Empty type list
template <>
class varvector<void> {
public:
	template <typename Visitor>
	void forall(Visitor &) {
	}
};

template <typename Current, typename Next>
class varvector<std::pair<Current, Next> > : public varvector<Next> {
	typedef varvector<Next> p_t;
	std::vector<Current> items;
public:
	template <typename T>
	void push_back(T a, typename same<Current, T>::type * = 0) {
		items.push_back(a);
	}
	template <typename T>
	void push_back(T a, typename different<Current, T>::type * = 0) {
		p_t::push_back(a);
	}
	template <typename Visitor>
	void forall(Visitor & v) {
		for (size_t i = 0; i < items.size(); ++i) v(items[i]);
		p_t::forall(v);
	}
};

int main() {
	varvector<std::pair<int, std::pair<const char *, void> > > a;
	a.push_back(42);
	a.push_back("hello");
	a.push_back(42);
	a.push_back("hello");
	my_visitor v;
	a.forall(v);
	return 0;
}
