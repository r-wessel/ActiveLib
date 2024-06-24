
# Container

Common STL containers specialised to hold polymorphic objects without slicing, including:

| ActiveLib | Specialisation of |
| --- | --- |
| HashMap | std::unordered_map |
| List | std::list |
| Map | std::map |
| MultiMap | std::multi_map |
| Vector | std::vector |

This is accomplished by:

1. Wrapping a contained object in a `unique_ptr`
2. Requiring that the stored class complies with the `Clonable` interface;

> Note: The same can be done without any additional coding by using `shared_ptr` instead of `unique_ptr`. These modules are for cases where `shared_ptr` use is not appropriate. Use of object cloning combined with `unique_ptr` also leaves no ambiguity about ownership:
> - If the container is copied, the objects held by the original and the copy are independent (the default behaviour for a container of `shared_ptr` is for both to reference the same objects).
> - If a contained object is assigned elsewhere, e.g. to another variable, it requires an explicit decision about resource ownership. `unique_ptr` will not allow itself to be simply copied, so either the object has to be released from the container, i.e. removed and ownsership reassigned, or it must be referenced through a pointer (using `unique_ptr::get()`).

Object resource management can be further optimised by implementing the `Mover` protocol in classes to be used with these container (to optimise move operations).

Note that none of the container classes require non-null entries. It is entirely up the implementor if this rule should be adopted or enforced.

## Discussion

**Pros**
- Convenient polymorphic container utilising all the features of the existing STL containers
- No manual memory management, e.g. ownership is not passed by raw pointer, no bare new/delete
- Object ownership is clear
- Clonable compliance allows the container to act as a if it holds values, e.g. it's copyable

**Cons**
- All objects must be allocated on the heap (increases possibility of heap fragmentation)
- Objects must implement the Clonable protocol - might be an obstacle when sub-classing 3rd-party objects

## Example

The following code creates a `Vector` of objects subclassed from a `Base` class, copies the vector into a new `Vector` and then prints messages demonstrating the content of both the original copied containers:

```Cpp
	class Base : public Cloner {
	public:
		Base* clonePtr() const override = 0;
		virtual void message() = 0;
	};
	
	class Foo : public Base {
	public:
		Foo* clonePtr() const override { return new Foo{*this}; }
		void message() override { std::cout << "Foo"; };
	};
	
	class Bar : public Base {
	public:
		Bar* clonePtr() const override { return new Bar{*this}; }
		void message() override { std::cout << "Bar"; };
	};
		//Populate a container with some subclass instances
	Vector<Base> container;
	container.emplace_back(std::make_unique<Foo>());
	container.emplace_back(std::make_unique<Bar>());
		//Make an independent copy of the container
	auto copy = container;
		//Print the original container messages
	for (auto& item : container)
		item->message();
		//Print the copy container messages
	for (auto& item : copy)
		item->message();
```

> Prints:
> 
> 	FooBarFooBar  
