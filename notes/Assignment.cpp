// TEST initialization vs copy initialization vs copy

// Must call T(const T &)
T t = T();

// Must call T(...)
T t();

// Must call operator=(const T &)
T t;
t = T();