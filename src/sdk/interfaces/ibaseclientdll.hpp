#pragma once
#include <cstdint>

struct RecvProp;

struct DVariant {
	union
	{
		float m_Float;
		long m_Int;
		char *m_pString;
		void *m_pData;
		float m_Vector[3];
		int64_t m_Int64;
	};

	int m_Type;
};

struct CRecvProxyData {
	const RecvProp* m_pRecvProp;
	DVariant m_Value;
	int m_iElement;
	int m_ObjectID;
};

struct RecvTable{
	RecvProp *m_pProps;
	int m_nProps;
	void *m_pDecoder;
	char *m_pNetTableName;
	bool m_bInitialized;
	bool m_bInMainList;
};

typedef void (*RecvVarProxyFn) (const CRecvProxyData *pData, void *pStruct, void *pOut);

enum SendPropType {
	DPT_Int=0,
	DPT_Float,
	DPT_Vector,
	DPT_VectorXY, // Only encodes the XY of a vector, ignores Z
	DPT_String,
	DPT_Array,	// An array of the base types (can't be of datatables).
	DPT_DataTable
};

struct RecvProp {
	char *m_pVarName;
	SendPropType m_RecvType;
	int m_Flags;
	int m_StringBufferSize;
	bool m_bInsideArray;
	const void *m_pExtraData;
	RecvProp *m_pArrayProp;
	void *m_ArrayLengthProxy;
	RecvVarProxyFn m_ProxyFn;
	void *m_DataTableProxyFn;
	RecvTable *m_pDataTable;
	int m_Offset;
	int m_ElementStride;
	int m_nElements;
	const char *m_pParentArrayPropName;
};

class IClientNetworkable;
typedef IClientNetworkable* (*CreateClientClassFn)(int entnum, int serialNum);
typedef IClientNetworkable* (*CreateEventFn)();

class ClientClass {
public:
	CreateClientClassFn m_pCreateFn;
	CreateEventFn *m_pCreateEventFn;
	char* m_pNetworkName;
	RecvTable *m_pRecvTable;
	ClientClass* m_pNext;
	int m_ClassID;
};

class IBaseClientDLL {
public:
    VFUNC(ClientClass*, getAllClasses, 8, (), (this))
};