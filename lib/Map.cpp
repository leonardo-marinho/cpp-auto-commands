#ifndef Map_cpp
#define Map_cpp

template <typename TKey, typename TValue>
class Map
{
private:
  struct KeyValue
  {
    TKey m_key;
    TValue m_value;
  };

  const static int MAX_ITEMS = 10;
  KeyValue m_keyValuePairs[MAX_ITEMS];
  int m_itemCount = 0;

public:
  bool add(TKey t_key, TValue t_value)
  {
    if (m_itemCount < MAX_ITEMS && !hasKey(t_key))
    {
      m_keyValuePairs[m_itemCount].m_key = t_key;
      m_keyValuePairs[m_itemCount].m_value = t_value;
      m_itemCount++;
      return true;
    }
    return false;
  }

  bool setValue(TKey t_key, TValue t_value)
  {
    for (int i = 0; i < m_itemCount; i++)
    {
      if (m_keyValuePairs[i].m_key == t_key)
      {
        m_keyValuePairs[i].m_value = t_value;
        return true;
      }
    }
    return false; // A chave não foi encontrada
  }

  bool remove(TKey t_key)
  {
    for (int i = 0; i < m_itemCount; i++)
    {
      if (m_keyValuePairs[i].m_key == t_key)
      {
        m_keyValuePairs[i] = m_keyValuePairs[m_itemCount - 1];
        m_itemCount--;
        return true;
      }
    }
    return false;
  }

  TKey getKeyByIndex(int t_index)
  {
    if (t_index < m_itemCount)
    {
      return m_keyValuePairs[t_index].m_key;
    }

    return -1;
  }

  TValue getValue(TKey t_key)
  {
    for (int i = 0; i < m_itemCount; i++)
    {
      if (m_keyValuePairs[i].m_key == t_key)
      {
        return m_keyValuePairs[i].m_value;
      }
    }
    return TValue();
  }

  String getKeysString()
  {
    String keys = "";
    for (int i = 0; i < m_itemCount; i++)
    {
      keys += "0x" + String(m_keyValuePairs[i].m_key, HEX);
      if (i < m_itemCount - 1)
      {
        keys += ", ";
      }
    }
    return keys;
  }

  bool hasKey(TKey t_key)
  {
    for (int i = 0; i < m_itemCount; i++)
    {
      if (m_keyValuePairs[i].m_key == t_key)
      {
        return true;
      }
    }

    return false;
  }

  TKey getLastKey()
  {
    return m_keyValuePairs[m_itemCount - 1].m_key;
  }

  TValue getLastValue()
  {
    return m_keyValuePairs[m_itemCount - 1].m_value;
  }

  int getItemCount()
  {
    return m_itemCount;
  }
};

#endif