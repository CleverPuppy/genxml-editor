#include "xml_types.h"
#include <memory>

bool ModalOKButton();
bool ModalCancelButton();

class XMLEditValueUI
{
public:
	void Render();
	XMLValueData currentEditing;
private:
	bool bHaveInfo = false;
};

class XMLEditEnumUI
{
public:
	void Render();
	XMLEnumData currentEditing;
private:
	bool bHavePrefix = false;
	bool bHaveInfo = false;
	XMLEditValueUI valueEdiotr;
};

class XMLEditFieldUI
{
public:
	void Render();
	XMLFieldData currentEditing;
	XMLEditValueUI valueEditor;
private:
	bool bHaveInfo;
	bool bHavePrefix;
};

class XMLEditStructUI
{
public:
	void Render();
	XMLStructData currentEditing;
private:
	bool bHaveInfo;
	XMLEditFieldUI fieldEditor;
};
