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

void wxDragListBox::OnLeftUp(wxMouseEvent& event)
{
	if(m_readyToDrag){
		wxString draggedItem = GetString(m_draggedItemIndex);
		Delete(m_draggedItemIndex);
		Insert(draggedItem, m_dropIndex);
		Deselect(GetSelection());

		m_gotFirstItem = false;
		m_readyToDrag = false;
	}

    event.Skip();
}

void wxDragListBox::OnMouseMove(wxMouseEvent& event)
{
	if(event.LeftIsDown()){
		if(!m_gotFirstItem){
			m_draggedItemIndex = GetSelection();

			if(m_draggedItemIndex != wxNOT_FOUND){
				m_gotFirstItem = true;
			}
		} else {
			m_dropIndex = GetSelection();

			if(m_dropIndex != wxNOT_FOUND && m_dropIndex != m_draggedItemIndex){
				m_readyToDrag = true;
			} else {
				m_readyToDrag = false;
			}
		}
	}

    event.Skip();	
}