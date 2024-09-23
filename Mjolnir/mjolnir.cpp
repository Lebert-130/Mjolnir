#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include <wx/listbox.h>
#include <wx/dnd.h>
#include <wx/mdi.h>
#include <wx/glcanvas.h>
#include <wx/splitter.h>
#include <wx/splash.h>
#include <wx/aui/aui.h>
#include <wx/bookctrl.h>
#include <wx/propdlg.h>

#include "mjolnir.h"
#include "fgd.h"
#include "map.h"

CFgd testFGD;
wxToolBar* tbar;
wxChoice* entityChoice;
MapFrame* frame;
wxMenuBar* menuBar;

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

MapFrame::MapFrame(const wxString& title)
: wxMDIParentFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600))
{
	wxIcon icon = wxICON(appicon);
	SetIcon(icon);

	m_mgr.SetManagedWindow(this);

	testFGD.LoadFGD("Test.fgd");

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

	wxTextCtrl* textCtrl1 = new wxTextCtrl(this, wxID_ANY, "Pane 1");

	m_mgr.AddPane(textCtrl1, wxAuiPaneInfo().Right().Caption("Texture group").BestSize(200,100).Position(0));
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
	timer->Start(50);

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
	m_leftSplitter->SetSashGravity(0.18);
	m_leftSplitter->SplitHorizontally(glCanvasPersp, scrollFront);

	m_rightSplitter->SetMinimumPaneSize(100);
	m_rightSplitter->SetSashGravity(0.18);
	m_rightSplitter->SplitHorizontally(scrollTop, scrollSide);

	mainSplitter->SetMinimumPaneSize(100);
	m_rightSplitter->SetSashGravity(0.18);
	mainSplitter->SplitVertically(m_leftSplitter, m_rightSplitter);

	wxSizer *sizerPersp = new wxBoxSizer(wxHORIZONTAL);
	glCanvasPersp->SetSizer(sizerPersp);

    m_leftSplitter->Connect(wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED, wxSplitterEventHandler(MapDoc::OnLeftSplitterSashChanged), NULL, this);
    m_rightSplitter->Connect(wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED, wxSplitterEventHandler(MapDoc::OnRightSplitterSashChanged), NULL, this);
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

			resetTimer->Start(100, wxTIMER_ONE_SHOT);
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

ObjectPropertiesSheetDialog::ObjectPropertiesSheetDialog(wxWindow* parent)
{
	Create(parent, wxID_ANY, "Object Properties",
		wxDefaultPosition, wxDefaultSize,
		wxDEFAULT_DIALOG_STYLE);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	wxBookCtrlBase* bookCtrl = GetBookCtrl();

	wxPanel* objectPage = new wxPanel(bookCtrl);

	bookCtrl->AddPage(objectPage, "Class Info", true);

	wxStaticText* classText = new wxStaticText(objectPage, wxID_ANY, "Class:");
	sizer->Add(classText, 0, wxALL, 2);

	wxArrayString entityChoices;
	testFGD.FGDToList(entityChoices, false);

	wxChoice* brushEntityChoice;
	brushEntityChoice = new wxChoice(objectPage, wxID_ANY, wxDefaultPosition, wxSize(100, -1), entityChoices);
	brushEntityChoice->Select(0);
	brushEntityChoice->Enable(false);
	sizer->Add(brushEntityChoice, 1, wxEXPAND | wxALL, 5);

	wxStaticText* attributesText = new wxStaticText(objectPage, wxID_ANY, "Attributes:");
	sizer->Add(attributesText, 0, wxALL, 2);

	wxListBox* attributesBox = new wxListBox(objectPage, wxID_ANY, wxPoint(10,10), wxSize(260, 120));
	std::vector<std::string> allAttributes = scMap[std::string(brushEntityChoice->GetString(brushEntityChoice->GetSelection()))].allAttributes;
	for (int i = 0; i < allAttributes.size(); i++){
		attributesBox->Append(allAttributes[i]);
	}

	sizer->Add(attributesBox, 0, wxALL, 5);

	objectPage->SetSizer(sizer);

	sizer->Fit(objectPage);

	LayoutDialog();
}

bool MjolnirApp::OnInit()
{
	wxBitmap splashScreen;

	if(splashScreen.LoadFile("splashscreen", wxBITMAP_TYPE_RESOURCE)){
		wxSplashScreen* splash = new wxSplashScreen(splashScreen,
			wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT, 2000, NULL, -1,
			wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxSTAY_ON_TOP);
	}

	Sleep(2000);

	frame = new MapFrame("Mjolnir");
	frame->Maximize();
	frame->Show(true);

	return true;
}