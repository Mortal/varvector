///////////////////////////////////////////////////////////////////////////////
// Type variadic vector.
// Mathias Rav, Jungwoo Yang, November 2, 2012.
///////////////////////////////////////////////////////////////////////////////

#include <iostream> // for my_visitor, using std::cout
#include <vector> // type non-variadic vector basis
#include <stdexcept>

// Variadic type vector visitor concept:
// Must have a operator() that is templated to accept
// any of the types used in the vector
class my_visitor {
public:
	typedef void result_type;

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
	void push_back(T a, typename same<Current, T>::type * = 0) {
		items.push_back(std::make_pair(this->size(), a));
		++this->m_size;
	}
	template <typename T>
	void push_back(T a, typename different<Current, T>::type * = 0) {
		p_t::push_back(a);
	}
};

int main() {
	varvector<std::pair<int, std::pair<const char *, void> > > a;
	a.push_back(42);
	a.push_back("hello");
	a.push_back(42);
	a.push_back("hello");
	my_visitor v;
	for (auto i = a.begin(); i != a.end(); ++i) {
		i(v);
	}
	return 0;
}
