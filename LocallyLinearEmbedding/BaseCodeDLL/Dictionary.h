#pragma once

template <class K, class V>  
class Dictionary
{
public:
	Dictionary();
	~Dictionary();
	void FreeMemory();
	void Add(K key, V value);
	bool ContainsKey(K key) const;
	Vector<K> Keys() const;

	__forceinline V& operator[] (K key)
	{
		if (ContainsKey(key))
			return _data[key];
		else
			throw out_of_range("Key not in dictionary");
	}

	__forceinline const V& operator[] (K key) const
	{
		if (ContainsKey(key))
		{
			return _data.find(key)->second;
		} else
			throw out_of_range("Key not in dictionary");
	}




private:
	map<K,V> _data;
};

template <class K, class V>
Dictionary<K,V>::Dictionary()
{
	
}

template <class K, class V>
Dictionary<K,V>::~Dictionary()
{
	FreeMemory();
}

template <class K, class V>
void Dictionary<K,V>::FreeMemory()
{
	_data.clear();
}

template <class K, class V>
void Dictionary<K,V>::Add(K key, V value)
{
	if (ContainsKey(key))
		throw invalid_argument("Key already exists in Dictionary");
	_data[key] = value;
}


template <class K, class V>
bool Dictionary<K,V>::ContainsKey(K key) const
{
	return _data.find(key) != _data.end();
}

template <class K, class V>
Vector<K> Dictionary<K,V>::Keys() const
{
	Vector<K> keys;
	for (map<K,V>::const_iterator it = _data.begin(); it != _data.end(); it++)
	{
		keys.PushEnd(it->first);
	}
	return keys;
}

