#ifndef Mcp_cpp
#define Mcp_cpp

#include <mcp2515.h>
#include "./Map.cpp"

class MCP
{
private:
  MCP2515 mcp2515;
  Map<canid_t, can_frame> m_watchList;
  Map<canid_t, void (*)(MCP &, can_frame)> m_onReceiveCallbackMap;
  struct can_frame canMsg;
  bool m_debugAll = false;
  bool m_watchAll = false;
  unsigned long m_lastDebugPaginatedTimestamp = 0;
  int m_debugPage = 1;
  int m_debugItemsPerPage = 20;

  void updateWatchList(can_frame t_newCanMsg)
  {
    if (m_watchAll && !m_watchList.hasKey(t_newCanMsg.can_id))
    {
      m_watchList.add(t_newCanMsg.can_id, t_newCanMsg);
    }
    else if (m_watchList.hasKey(t_newCanMsg.can_id))
    {
      m_watchList.setValue(t_newCanMsg.can_id, t_newCanMsg);
    }
  }

  void
  callCallback(can_frame t_newCanMsg)
  {
    if (m_onReceiveCallbackMap.hasKey(t_newCanMsg.can_id))
    {
      m_onReceiveCallbackMap.getValue(t_newCanMsg.can_id)(*this, t_newCanMsg);
    }
  }

public:
  MCP(const uint8_t t_cs)
      : mcp2515(t_cs),
        m_onReceiveCallbackMap()
  {
  }

  MCP &begin(const CAN_SPEED t_canSpeed, const CAN_CLOCK t_canClock)
  {
    mcp2515.reset();
    mcp2515.setBitrate(t_canSpeed, t_canClock);
    mcp2515.setNormalMode();

    return *this;
  }

  void loop()
  {
    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK)
    {
      updateWatchList(canMsg);
      callCallback(canMsg);

      if (m_debugAll)
      {
        debugCanMsg(canMsg);
      }
    } 
  }

  void debugCanMsg(can_frame t_canMsg)
  {
    String id = String(t_canMsg.can_id, HEX);
    if (id.length() == 2)
      Serial.print("0x0" + id);
    else
      Serial.print("0x" + id);
    Serial.print(" | ");
    Serial.print(t_canMsg.can_dlc, HEX);
    Serial.print(" | ");
    for (int i = 0; i < t_canMsg.can_dlc; i++)
    {
      String val = String(t_canMsg.data[i], HEX);
      if (val.length() == 1)
        Serial.print("0" + val);
      else
        Serial.print(val);
      Serial.print(" ");
    }
    Serial.println();
  }

  void debugAll()
  {
    m_debugAll = true;
  }

  void stopDebugAll()
  {
    m_debugAll = false;
  }

  can_frame getValue(canid_t t_id)
  {
    return m_watchList.getValue(t_id);
  }

  MCP &watch(canid_t t_id)
  {
    m_watchList.add(t_id, can_frame());
    return *this;
  }

  MCP &watchAll()
  {
    m_watchAll = true;
    return *this;
  }

  MCP &stopWatchAll()
  {
    m_watchAll = false;
    return *this;
  }

  MCP &onReceiveCallback(void (*t_onMsgReceive)(MCP &, can_frame))
  {
    canid_t canId = m_watchList.getLastKey();
    m_onReceiveCallbackMap.add(canId, t_onMsgReceive);
    return *this;
  }

  void stopWatch(canid_t t_id)
  {
    m_watchList.remove(t_id);
  }

  void send(can_frame t_msg)
  {
    mcp2515.sendMessage(&t_msg);
  }

  bool isOK()
  {
    return mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK;
  }

  static can_frame createEmptyMsg(canid_t t_id, __u8 t_canDlc)
  {
    can_frame msg;
    msg.can_id = t_id;
    msg.can_dlc = t_canDlc;
    return MCP::resetMsgData(msg);
  }

  static can_frame resetMsgData(can_frame t_msg)
  {
    for (int i = 0; i < t_msg.can_dlc; i++)
    {
      t_msg.data[i] = 0;
    }
    return t_msg;
  }

  void setDebugItemsPerPage(int t_debugItemsPerPage)
  {
    m_debugItemsPerPage = t_debugItemsPerPage;
  }

  void setDebugPage(int t_debugPage)
  {
    m_debugPage = t_debugPage;
  }

  void debugPaginated()
  {
    if (millis() - m_lastDebugPaginatedTimestamp < 500)
    {
      return;
    }

    m_lastDebugPaginatedTimestamp = millis();

    int idsCount = m_watchList.getItemCount();
    canid_t ids[m_debugItemsPerPage];

    int startIndex = (m_debugPage - 1) * m_debugItemsPerPage;
    int endIndex = startIndex + m_debugItemsPerPage - 1;
    if (endIndex > idsCount)
    {
      endIndex = idsCount - 1;
    }

    Serial.println("Index from " + String(startIndex) + " to " + String(endIndex) + " of " + String(idsCount) + " items");

    for (int i = startIndex; i < endIndex; i++)
    {
      ids[i] = m_watchList.getKeyByIndex(i);
    }

    for (int i = 0; i < m_debugItemsPerPage; i++)
    {
      for (int j = i + 1; j < m_debugItemsPerPage; j++)
      {
        if (ids[i] > ids[j])
        {
          canid_t temp = ids[i];
          ids[i] = ids[j];
          ids[j] = temp;
        }
      }
    }

    float totalPages = idsCount / m_debugItemsPerPage + 0.5;

    Serial.println("PAGE: " + String(m_debugPage) + "" + " | ITEMS PER PAGE: " + String(m_debugItemsPerPage) + " | TOTAL: " + String(idsCount));

    for (int i = 0; i < m_debugItemsPerPage; i++)
    {
      debugCanMsg(m_watchList.getValue(ids[i]));
    }

    Serial.println();
  }

  String getIdsString()
  {
    return m_watchList.getKeysString();
  }
};

#endif