#pragma once

#include "stdafx.h"

typedef struct _cbNeverChanges
{
	XMFLOAT4X4 Projection;
} cbNeverChanges;

typedef struct _cbChangesEveryFrame
{
	XMFLOAT4X4  View;

	XMFLOAT3    Light;
	FLOAT       dummy; // É_É~Å[
} cbChangesEveryFrame;

typedef struct _cbChangesEveryObject
{
	XMFLOAT4X4 World;
} cbChangesEveryObject;

enum CONST_BUFFER_TYPE
{
	CONST_BUFFER_TYPE_NEVER_CHANGE,
	CONST_BUFFER_TYPE_EVERY_FRAME,
	CONST_BUFFER_TYPE_EVERY_OBJECT,
};
