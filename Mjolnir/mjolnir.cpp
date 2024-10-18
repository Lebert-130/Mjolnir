#include <algorithm>
#include <math.h>

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include <wx/listbox.h>
#include <wx/dnd.h>
#include <wx/mdi.h>
#include <wx/glcanvas.h>
#include <wx/dcbuffer.h>
#include <wx/splitter.h>
#include <wx/splash.h>
#include <wx/aui/aui.h>
#include <wx/bookctrl.h>
#include <wx/propdlg.h>

#include "mjolnir.h"
#include "fgd.h"
#include "map.h"
#include "wad3.h"

#define IMAGE_SIZE 128

CFgd testFGD;
wxToolBar* tbar;
wxChoice* entityChoice;
MapFrame* frame;
wxMenuBar* menuBar;

wxComboBox* comboBox;

int currentTexture = 0;

IMPLEMENT_APP(MjolnirApp)

AboutDialog::AboutDialog(wxWindow* parent)
: wxDialog(parent, wxID_ANY, wxT("Mjolnir"), wxDefaultPosition, wxSize(300, 200))
{
	wxPanel* panel = new wxPanel(this, wxID_ANY);
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	wxBitmap mappingIcon = wxBITMAP(bettericon);

	wxStaticBitmap* iconBitmapCtrl = new wxStaticBitmap(panel, wxID_ANY, mappingIcon);
	sizer->Add(iconBitmapCtrl, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

	wxStaticText* aboutText = new wxStaticText(panel, wxID_ANY, wxT("Mjolnir map editor made by Lebert © 2021-2024\nIcons by: J. Flames.\n\nVersion 1.0"));
	sizer->Add(aboutText, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

	wxButton* okButton = new wxButton(panel, wxID_OK, wxT("OK"));
	sizer->Add(okButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

	panel->SetSizer(sizer);

	sizer->Fit(panel);
	sizer->SetSizeHints(this);

	okButton->SetDefault();
}

TextureBrowserDialog::TextureBrowserDialog(wxWindow* parent, const wxString& title)
: wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(900, 600))
{
	wxPanel* panel = new wxPanel(this, wxID_ANY);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxArrayString choices;
	choices.Add("Option 1");
	choices.Add("Option 2");
	choices.Add("Option 3");
	wxChoice* choiceControl = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);
	mainSizer->Add(choiceControl, 0, wxALL | wxEXPAND, 5);

	m_listCtrl = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxSize(800, 400), wxLC_ICON | wxLC_SINGLE_SEL);
	m_listCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(TextureBrowserDialog::OnItemSelected), nullptr, this);
	mainSizer->Add(m_listCtrl, 1, wxEXPAND | wxALL, 10);

	InitImageList();

	m_listCtrl->SetBackgroundColour(wxColour(0, 0, 0));
	m_listCtrl->SetTextColour(wxColour(255, 255, 255));

	for (int i = 0; i < textures.size(); i++){
		m_listCtrl->InsertItem(i, textures[i].name, i);
		m_listCtrl->SetItemBackgroundColour(i, wxColour(0,0,255));
	}

	wxPanel* bottomPanel = new wxPanel(panel, wxID_ANY);
	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);

	wxCheckBox* usedTexturesCheckbox = new wxCheckBox(bottomPanel, wxID_ANY, "Only used textures");
	bottomSizer->Add(usedTexturesCheckbox, 0, wxALL, 5);

	wxCheckBox* animateTexturesCheckbox = new wxCheckBox(bottomPanel, wxID_ANY, "Animate textures");
	bottomSizer->Add(animateTexturesCheckbox, 0, wxALL, 5);

	wxTextCtrl* filterTextbox = new wxTextCtrl(bottomPanel, wxID_ANY, "", wxDefaultPosition, wxSize(87, 20));
	bottomSizer->Add(filterTextbox, 0, wxALL, 5);

	wxButton* selectButton = new wxButton(bottomPanel, wxID_ANY, "Replace...");
	bottomSizer->Add(selectButton, 0, wxALL, 5);

	bottomPanel->SetSizer(bottomSizer);
	mainSizer->Add(bottomPanel, 0, wxALL, 5);

	panel->SetSizer(mainSizer);
	mainSizer->Fit(this);
}

void TextureBrowserDialog::OnItemSelected(wxListEvent& event)
{
	int itemIndex = event.GetIndex();
	currentTexture = itemIndex;

	Close();
}

void TextureBrowserDialog::InitImageList()
{
	m_imageList = new wxImageList(IMAGE_SIZE, IMAGE_SIZE, true);

	for (int i = 0; i < textures.size(); i++){
		wxImage img = textures[i].guiImage;

		if (img.GetWidth() != IMAGE_SIZE || img.GetHeight() != IMAGE_SIZE)
			img = img.Rescale(IMAGE_SIZE, IMAGE_SIZE);

		m_imageList->Add(wxBitmap(img));
	}

	m_listCtrl->AssignImageList(m_imageList, wxIMAGE_LIST_NORMAL);
}

MapFrame::MapFrame(const wxString& title)
: wxMDIParentFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600))
{
	wxIcon icon = wxICON(appicon);
	SetIcon(icon);

	m_mgr.SetManagedWindow(this);

	testFGD.LoadFGD("Test.fgd");
	LoadWadFile("test.wad");

	visgroupPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	wxDragListBox* visgroupListBox = new wxDragListBox(visgroupPanel);
	visgroupListBox->AddItem("VisGroup 1");
	visgroupListBox->AddItem("VisGroup 2");
	visgroupListBox->AddItem("VisGroup 3");
	visgroupListBox->AddItem("VisGroup 4");

	objectPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	wxArrayString entityChoices;
	testFGD.FGDToList(entityChoices, true);
	entityChoice = new wxChoice(objectPanel, wxID_ANY, wxDefaultPosition, wxSize(100,-1), entityChoices);
	entityChoice->Select(0);

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(entityChoice, 1, wxEXPAND | wxALL, 5);
	objectPanel->SetSizer(sizer);
	sizer->Fit(objectPanel);

	texturePanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	wxButton* browseButton = new wxButton(texturePanel, wxID_ANY, wxT("Browse..."), wxPoint(20, 20));
	Connect(browseButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MapFrame::OnShowDialog));

	m_mgr.AddPane(texturePanel, wxAuiPaneInfo().Right().Caption("Texture group").BestSize(200, 100).Position(0));
	m_mgr.AddPane(visgroupPanel, wxAuiPaneInfo().Right().Caption("VisGroups").BestSize(200,100).Position(1));
	m_mgr.AddPane(objectPanel, wxAuiPaneInfo().Right().Caption("Objects").BestSize(200,100).Position(2));

	m_mgr.Update();

	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(ID_New, "&New\tCtrl-N", "Create a new child window");
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, "E&xit\tAlt-F4", "Quit this program");

	wxMenu* toolsMenu = new wxMenu();
	toolsMenu->Append(ID_Options, "&Options\tF2", "Options");

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(wxID_ABOUT, "&About\tF1", "Show about dialog");

	menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "&File");
	menuBar->Append(toolsMenu, "&Tools");
	menuBar->Append(helpMenu, "&Help");
	
	SetMenuBar(menuBar);

	CreateStatusBar(2);
	SetStatusText("The power of Thor");

	tbar = new wxToolBar(this, -1, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL);

	wxBitmap selectionIcon = wxBITMAP(select);
	wxBitmap zoomIcon = wxBITMAP(zoom);
	wxBitmap cameraIcon = wxBITMAP(camera);
	wxBitmap entityIcon = wxBITMAP(addentity);
	wxBitmap createBlock = wxBITMAP(addblock);
	wxBitmap paintFace = wxBITMAP(paintface);
	wxBitmap paintBlock = wxBITMAP(paintblock);
	wxBitmap blockSplit = wxBITMAP(blocksplit);

	tbar->SetToolBitmapSize(wxSize(32,32));
	tbar->AddTool(TOOL_SELECT, _("Selection"), selectionIcon, wxT("Selection Tool"), wxITEM_RADIO);
	tbar->AddTool(TOOL_ZOOM, _("Zoom"), zoomIcon, wxT("Zoom Tool"), wxITEM_RADIO);
	tbar->AddTool(TOOL_CAMERA, _("Camera"), cameraIcon, wxT("Camera"), wxITEM_RADIO);
	tbar->AddTool(TOOL_ENTITY, _("Entity Tool"), entityIcon, wxT("Entity Tool"), wxITEM_RADIO);
	tbar->AddTool(TOOL_CBLOCK, _("Create Block"), createBlock, wxT("Create Block"), wxITEM_RADIO);
	tbar->AddTool(TOOL_SBLOCK, _("Split Block"), blockSplit, wxT("Split Block"), wxITEM_RADIO);
	tbar->AddTool(TOOL_PFACE, _("Paint Face"), paintFace);
	tbar->AddTool(TOOL_PBLOCK, _("Paint Block"), paintBlock);
	tbar->Realize();
	SetToolBar(tbar);
}

void MapFrame::OnShowDialog(wxCommandEvent& event)
{
	TextureBrowserDialog dialog(this, wxT("Textures"));
	dialog.ShowModal();
}

MapFrame::~MapFrame()
{
	m_mgr.UnInit();
}

BEGIN_EVENT_TABLE(MapFrame, wxMDIParentFrame)
	EVT_MENU(ID_New, MapFrame::OnNew)
	EVT_MENU(ID_Options, MapFrame::OnOptions)
	EVT_MENU(wxID_EXIT, MapFrame::OnExit)
	EVT_MENU(wxID_ABOUT, MapFrame::OnAbout)
	EVT_MENU(ID_Map, MapFrame::OnMap)
END_EVENT_TABLE()

void MapFrame::OnNew(wxCommandEvent& WXUNUSED(event))
{
	MapDoc* subframe = new MapDoc(this, "New Document");
	subframe->Show(true);
}

void MapFrame::OnOptions(wxCommandEvent& WXUNUSED(event))
{
	OptionsPropertySheetDialog* dlg = new OptionsPropertySheetDialog(NULL);
	dlg->ShowModal();
	dlg->Destroy();
}

void MapFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void MapFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	AboutDialog aboutDialog(NULL);
	aboutDialog.ShowModal();
}

void MapFrame::OnMap(wxCommandEvent& WXUNUSED(event))
{
	ObjectPropertiesSheetDialog* dlg = new ObjectPropertiesSheetDialog(NULL);
	dlg->ShowModal();
	dlg->Destroy();
}

MapDoc::MapDoc(wxMDIParentFrame* parent, const wxString& title)
: wxMDIChildFrame(parent, wxID_ANY, title)
{
	wxMenu* mapMenu = new wxMenu();
	mapMenu->Append(ID_Map, "Map Properties...");

	menuBar->Append(mapMenu, "&Map");

	enterKeyHandled = false;

	timer = new wxTimer(this, 1337);
	resetTimer = new wxTimer(this, 1338);
	timer->Start(30);

	wxSplitterWindow *mainSplitter = new wxSplitterWindow(this);
	m_leftSplitter = new wxSplitterWindow(mainSplitter);
	m_rightSplitter = new wxSplitterWindow(mainSplitter);

	MapView3D *glCanvasPersp = new MapView3D(m_leftSplitter);

	wxScrolledWindow* scrollTop = new wxScrolledWindow(m_rightSplitter);
	glCanvasTop = new MapView2D(scrollTop);
	glCanvasTop->SetRenderMode(VIEW_TOP);

	wxScrolledWindow* scrollFront = new wxScrolledWindow(m_leftSplitter);
	glCanvasFront = new MapView2D(scrollFront);
	glCanvasFront->SetRenderMode(VIEW_FRONT);

	wxScrolledWindow* scrollSide = new wxScrolledWindow(m_rightSplitter);
	glCanvasSide = new MapView2D(scrollSide);
	glCanvasSide->SetRenderMode(VIEW_SIDE);

	m_leftSplitter->SetMinimumPaneSize(100);
	m_leftSplitter->SetSashGravity(0.47);
	m_leftSplitter->SplitHorizontally(glCanvasPersp, scrollFront);

	m_rightSplitter->SetMinimumPaneSize(100);
	m_rightSplitter->SetSashGravity(0.47);
	m_rightSplitter->SplitHorizontally(scrollTop, scrollSide);

	mainSplitter->SetMinimumPaneSize(100);
	mainSplitter->SetSashGravity(0.47);
	mainSplitter->SplitVertically(m_leftSplitter, m_rightSplitter);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(mainSplitter, 1, wxEXPAND);
	SetSizer(sizer);

    m_leftSplitter->Connect(wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED, wxSplitterEventHandler(MapDoc::OnLeftSplitterSashChanged), NULL, this);
    m_rightSplitter->Connect(wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED, wxSplitterEventHandler(MapDoc::OnRightSplitterSashChanged), NULL, this);

	Layout();
}

MapDoc::~MapDoc()
{
	if(timer){
		timer->Stop();
		delete timer;
	}

	if(resetTimer){
		resetTimer->Stop();
		delete resetTimer;
	}
}

// Why the fuck I need to create TWO timers to just be able to handle the enter key?
// I have tried using EVT_KEY_DOWN, but for some reason when I press enter, the focus
// just changes to another view???

// And what just baffles me is that when I tried to look for a solution on the internet,
// I came across with the OFFICIAL WIKI, telling me to MAKE A TIMER as alternative to EVT_KEY_DOWN...

//TODO: Well... camera movement feels sluggish, maybe move all of the keyboard events to this timer at this point...
//Pressing enter feels much more comfortable though, so maybe this is the answer to the problem.
void MapDoc::OnTimer(wxTimerEvent& event)
{
	if(!enterKeyHandled){
		if(wxGetKeyState(WXK_RETURN)){
			SetFocus();
			glCanvasTop->EnterKey(true);
			glCanvasTop->Refresh();
			glCanvasFront->EnterKey(true);
			glCanvasFront->Refresh();
			glCanvasSide->EnterKey(true);
			glCanvasSide->Refresh();
			enterKeyHandled = true;

			resetTimer->Start(60, wxTIMER_ONE_SHOT);
		}
	}
}

void MapDoc::OnResetEnterKeyHandled(wxTimerEvent& event)
{
	glCanvasTop->EnterKey(false);
	glCanvasFront->EnterKey(false);
	glCanvasSide->EnterKey(false);
    enterKeyHandled = false;
	enterKeyCaptured = false;
}

void MapDoc::OnSize(wxSizeEvent& event)
{
	event.Skip();

	int width, height;
	GetClientSize(&width, &height);

	m_leftSplitter->SetSashPosition(height / 2, true);
	m_rightSplitter->SetSashPosition(height / 2, true);

	Layout();
}

void MapDoc::OnLeftSplitterSashChanged(wxSplitterEvent& event)
{
    int sashPosition = m_leftSplitter->GetSashPosition();
    m_rightSplitter->SetSashPosition(sashPosition);  // Sync the sash position with the right splitter
}

void MapDoc::OnRightSplitterSashChanged(wxSplitterEvent& event)
{
    int sashPosition = m_rightSplitter->GetSashPosition();
    m_leftSplitter->SetSashPosition(sashPosition);  // Sync the sash position with the left splitter
}

BEGIN_EVENT_TABLE(MapDoc, wxMDIChildFrame)
	EVT_TIMER(1337, MapDoc::OnTimer)
	EVT_TIMER(1338, MapDoc::OnResetEnterKeyHandled)
	EVT_SIZE(MapDoc::OnSize)
END_EVENT_TABLE()

OptionsPropertySheetDialog::OptionsPropertySheetDialog(wxWindow* parent)
{
	Create(parent, wxID_ANY, "Configure Mjolnir",
		wxDefaultPosition, wxDefaultSize,
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

	CreateButtons(wxOK | wxCANCEL);

	wxBookCtrlBase* bookCtrl = GetBookCtrl();

	wxPanel* generalPage = new wxPanel(bookCtrl);
	wxPanel* page2D = new wxPanel(bookCtrl);
	wxPanel* page3D = new wxPanel(bookCtrl);

	bookCtrl->AddPage(generalPage, "General", true);
	bookCtrl->AddPage(page2D, "2D views");
	bookCtrl->AddPage(page3D, "3D views");

	wxBoxSizer* generalSizer = new wxBoxSizer(wxVERTICAL);
	generalSizer->Add(new wxStaticText(generalPage, wxID_ANY, "General Settings"));
	generalPage->SetSizer(generalSizer);

	wxBoxSizer* sizer2D = new wxBoxSizer(wxVERTICAL);
	sizer2D->Add(new wxStaticText(page2D, wxID_ANY, "2D Settings"));
	page2D->SetSizer(sizer2D);

	wxBoxSizer* sizer3D = new wxBoxSizer(wxVERTICAL);
	sizer3D->Add(new wxStaticText(page3D, wxID_ANY, "3D Settings"));
	page3D->SetSizer(sizer3D);

	LayoutDialog();
}

AngleControl::AngleControl(wxWindow* parent, int initialAngle)
: wxPanel(parent), angle(initialAngle)
{
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	comboBox->SetValue("0");
	rotationTimer = new wxTimer(this, 1339);
}

AngleControl::~AngleControl()
{
	if (rotationTimer){
		rotationTimer->Stop();
		delete rotationTimer;
	}
}

void AngleControl::SetAngle(int parm_angle)
{
	angle = parm_angle;
	Refresh();
}

void AngleControl::OnTimer(wxTimerEvent& event)
{
	if (dragging){
		wxPoint mousePos = wxGetMousePosition();
		wxPoint clientPos = ScreenToClient(mousePos);

		int mouseX = clientPos.x;
		int mouseY = clientPos.y;

		wxSize size = GetClientSize();
		wxPoint center(size.GetWidth() / 2, size.GetHeight() / 2);

		float deltaX = mouseX - center.x;
		float deltaY = mouseY - center.y;

		angle = static_cast<int>(atan2(-deltaY, deltaX) * (180.0f / M_PI));
		angle = (angle + 360) % 360;

		SetAngle(angle);

		wxString angleStr = wxString::Format("%d", angle);
		comboBox->SetValue(angleStr);

		wxMouseState mouseState = wxGetMouseState();
		if (!mouseState.LeftDown()){
			rotationTimer->Stop();
			dragging = false;
		}
	}
}

void AngleControl::OnMouseLeftDown(wxMouseEvent& event)
{
	rotationTimer->Start(5);
	dragging = true;
}

void AngleControl::OnPaint(wxPaintEvent& event)
{
	wxBufferedPaintDC dc(this);
	PrepareDC(dc);

	//Why is it so difficult to make it transparent???
	//I had to hard-code the color because I just can't find a way to make it transparent so it grabs whatever color it is in any system.
	dc.SetBackground(wxBrush(wxColour(249,249,249)));
	dc.Clear();

	wxSize size = GetClientSize();

	int radius = std::min(size.GetWidth(), size.GetHeight()) / 2;
	wxPoint center(size.GetWidth() / 2, size.GetHeight() / 2);

	dc.SetBrush(*wxBLACK_BRUSH);
	dc.DrawCircle(center, radius);

	float radians = angle * (M_PI / 180.0f);
	int xEnd = center.x + static_cast<int>(radius * cos(radians));
	int yEnd = center.y - static_cast<int>(radius * sin(radians));

	dc.SetPen(wxPen(*wxRED, 1));
	dc.DrawLine(center.x, center.y, xEnd, yEnd);

	radians = (angle + 90) * (M_PI / 180.0f);
	xEnd = center.x + static_cast<int>(radius * cos(radians));
	yEnd = center.y - static_cast<int>(radius * sin(radians));

	dc.SetPen(wxPen(*wxGREEN, 1));
	dc.DrawLine(center.x, center.y, xEnd, yEnd);
}

BEGIN_EVENT_TABLE(AngleControl, wxPanel)
	EVT_PAINT(AngleControl::OnPaint)
	EVT_TIMER(1339, AngleControl::OnTimer)
	EVT_LEFT_DOWN(AngleControl::OnMouseLeftDown)
END_EVENT_TABLE()

ObjectPropertiesSheetDialog::ObjectPropertiesSheetDialog(wxWindow* parent)
{
	Create(parent, wxID_ANY, "Object Properties",
		wxDefaultPosition, wxDefaultSize,
		wxDEFAULT_DIALOG_STYLE);

	attributeChoices = NULL;
	attributeValues = NULL;

	isChoice = false;

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	wxBookCtrlBase* bookCtrl = GetBookCtrl();

	objectPage = new wxPanel(bookCtrl);

	bookCtrl->AddPage(objectPage, "Class Info", true);

	wxStaticText* classText = new wxStaticText(objectPage, wxID_ANY, "Class:");
	sizer->Add(classText, 0, wxALL, 2);

	wxArrayString entityChoices;
	testFGD.FGDToList(entityChoices, false);

	brushEntityChoice = new wxChoice(objectPage, wxID_ANY, wxDefaultPosition, wxSize(100, -1), entityChoices);
	brushEntityChoice->Select(0);
	brushEntityChoice->Enable(false);

	wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);
	rowSizer->Add(brushEntityChoice, 1, wxEXPAND | wxALL, 5);

	wxButton* smarteditButton = new wxButton(objectPage, wxID_ANY, "SmartEdit", wxDefaultPosition, wxDefaultSize, 0);
	rowSizer->Add(smarteditButton, 0, wxALL, 5);

	wxArrayString directions;
	directions.Add("Up");
	directions.Add("Down");

	comboBox = new wxComboBox(objectPage, wxID_ANY, "", wxDefaultPosition, wxSize(60, 30), directions, wxCB_DROPDOWN);
	rowSizer->Add(comboBox, 0, wxALL, 5);

	AngleControl* angleCtrl = new AngleControl(objectPage, 0);
	wxBoxSizer* angleSizer = new wxBoxSizer(wxVERTICAL);
	angleSizer->AddSpacer(-15);
	angleSizer->Add(angleCtrl, 0, wxALL, 5);
	angleCtrl->SetMinSize(wxSize(40, 40));
	rowSizer->Add(angleSizer, 0, wxALL, 5);

	sizer->Add(rowSizer, 0, wxEXPAND | wxALL, 1);

	wxStaticText* attributesText = new wxStaticText(objectPage, wxID_ANY, "Attributes:");
	sizer->Add(attributesText, 0, wxALL, 1);

	wxListBox* attributesBox = new wxListBox(objectPage, wxID_ANY, wxPoint(10,10), wxSize(260, 120));
	std::vector<Attribute> attributes = scMap[std::string(brushEntityChoice->GetString(brushEntityChoice->GetSelection()))].attributes;
	for (int i = 0; i < attributes.size(); i++){
		attributesBox->Append(attributes[i].description);
	}
	attributesBox->Connect(wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(ObjectPropertiesSheetDialog::OnListBoxSelect), nullptr, this);

	rowSizer2 = new wxBoxSizer(wxHORIZONTAL);
	rowSizer2->Add(attributesBox, 0, wxALL, 5);

	attributeValues = new wxTextCtrl(objectPage, wxID_ANY, "", wxPoint(10,10), wxSize(130, 20));
	rowSizer2->Add(attributeValues, 0, wxALL, 5);

	sizer->Add(rowSizer2, 0, wxEXPAND | wxALL, 1);

	objectPage->SetSizer(sizer);

	sizer->Fit(objectPage);

	LayoutDialog();
}

void ObjectPropertiesSheetDialog::OnListBoxSelect(wxCommandEvent& event)
{
	int selectedIndex = event.GetInt();
	wxString selectedItem = static_cast<wxListBox*>(event.GetEventObject())->GetString(selectedIndex);

	std::vector<Attribute> attributes = scMap[std::string(brushEntityChoice->GetString(brushEntityChoice->GetSelection()))].attributes;

	for (int i = 0; i < attributes.size(); i++){
		Attribute attr = attributes[i];

		if (attr.description == selectedItem){
			if (attr.choices.empty()){
				if (isChoice){
					attributeChoices->Destroy();
					attributeValues = new wxTextCtrl(objectPage, wxID_ANY, "", wxPoint(10, 10), wxSize(130, 20));
					rowSizer2->Add(attributeValues, 0, wxALL, 5);

					LayoutDialog();
					isChoice = false;
				}

				if (!attr.defaultvalue.empty())
					attributeValues->SetValue(attr.defaultvalue);
			}
			else {
				if (!isChoice){
					attributeValues->Destroy();
					wxArrayString choices;

					for (int j = 0; j < attr.choices.size(); j++){
						choices.Add(attr.choices[j]);
					}

					attributeChoices = new wxChoice(objectPage, wxID_ANY, wxPoint(10, 10), wxSize(120, 20), choices);
					attributeChoices->Select(atoi(attr.defaultvalue.c_str()));
					rowSizer2->Add(attributeChoices, 0, wxALL, 5);

					LayoutDialog();
					isChoice = true;
				}
			}

			return;
		}
	}
}

bool MjolnirApp::OnInit()
{
	wxBitmap splashScreen;

	if(splashScreen.LoadFile("splashscreen", wxBITMAP_TYPE_RESOURCE)){
		wxSplashScreen* splash = new wxSplashScreen(splashScreen,
			wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT, 2000, NULL, -1,
			wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	}

	Sleep(2000);

	frame = new MapFrame("Mjolnir");
	frame->Maximize();
	frame->Show(true);

	return true;
}