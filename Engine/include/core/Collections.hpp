namespace Disarray::Collections {

inline void for_each(auto& collection, auto&& func) { std::for_each(std::begin(collection), std::end(collection), func); }

} // namespace Disarray::Collections
