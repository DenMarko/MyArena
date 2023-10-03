#pragma once
#include "CSetting.h"
#include "ShowControlServer.h"

class CListServer : public CUIRender
{
public:
	CListServer();
	~CListServer();

public:
	virtual void OnAttach(bool *Is_Open) override {}
	virtual void OnDetach() override {}

	virtual void OnUIRender() override;
};

