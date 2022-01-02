#pragma once
#include <cstdint>

// Fuzion

#define EVENT_DEBUG_ID_INIT 42
#define EVENT_DEBUG_ID_SHUTDOWN 13

struct bf_read;
struct bf_write;

class IGameEvent;
class KeyValues;

class IGameEvent
{
public:
	virtual ~IGameEvent() {};
	virtual const char* getName() const = 0;

	virtual bool isReliable() const = 0;
	virtual bool isLocal() const = 0;
	virtual bool isEmpty(const char* keyname = nullptr) = 0;

	virtual bool getBool(const char* keyname = nullptr, bool default_value = false) = 0;
	virtual int getInt(const char* keyname = nullptr, int default_value = 0) = 0;
	virtual uint64_t getUint64(const char* keyname = nullptr, uint64_t default_value = 0) = 0;
	virtual float getFloat(const char* keyname = nullptr, float default_value = 0.0f) = 0;
	virtual const char* getString(const char* keyname = nullptr, const char* default_value = "") = 0;
	virtual const wchar_t* getWString(const char* keyname = nullptr, const wchar_t* default_value = L"") = 0;
	virtual const void* getPtr(const char* keyname = nullptr, const void* default_values = nullptr) = 0;

	virtual void setBool(const char* keyname, bool value) = 0;
	virtual void setInt(const char* keyname, int value) = 0;
	virtual void setUint64(const char* keyname, uint64_t value) = 0;
	virtual void setFloat(const char* keyname, float value) = 0;
	virtual void setString(const char* keyname, const char* value) = 0;
	virtual void setWString(const char* keyname, const wchar_t* value) = 0;
	virtual void setPtr(const char* keyname, const void* value) = 0;
};

class IGameEventListener2
{
public:
	virtual ~IGameEventListener2(void) {};

	virtual void fireGameEvent(IGameEvent *event) = 0;
	virtual int getEventDebugID(void) = 0;
};

class IGameEventManager2
{
public:
	virtual ~IGameEventManager2(void) {};
	virtual int loadEventsFromFile(const char* filename) = 0;
	virtual void reset() = 0;
	virtual bool addListener(IGameEventListener2* listener, const char* name, bool serverside) = 0;
	virtual bool findListener(IGameEventListener2* listener, const char* name) = 0;
	virtual void removeListener(IGameEventListener2* listener) = 0;
	virtual void addListenerGlobal(IGameEventListener2* listener, bool serverside) = 0;
	virtual IGameEvent* createEvent(const char* name, bool force = false, int* cookie = nullptr) = 0;
	virtual bool fireEvent(IGameEvent* event, bool bDontBroadcast = false) = 0;
	virtual bool fireEventClientSide(IGameEvent* event) = 0;
	virtual IGameEvent* duplicateEvent(IGameEvent* event) = 0;
	virtual void freeEvent(IGameEvent* event) = 0;
	virtual bool serializeEvent(IGameEvent* event, bf_write* buffer) = 0;
	virtual IGameEvent* unserializeEvent(bf_read* buffer) = 0;
	virtual KeyValues* getEventDataTypes(IGameEvent* event) = 0;
};
