/**********************************************************
 * Version $Id$
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    User Interface                     //
//                                                       //
//                    Program: SAGA                      //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                 WKSP_Data_Layers.cpp                  //
//                                                       //
//          Copyright (C) 2006 by Olaf Conrad            //
//                                                       //
//-------------------------------------------------------//
//                                                       //
// This file is part of 'SAGA - System for Automated     //
// Geoscientific Analyses'. SAGA is free software; you   //
// can redistribute it and/or modify it under the terms  //
// of the GNU General Public License as published by the //
// Free Software Foundation; version 2 of the License.   //
//                                                       //
// SAGA is distributed in the hope that it will be       //
// useful, but WITHOUT ANY WARRANTY; without even the    //
// implied warranty of MERCHANTABILITY or FITNESS FOR A  //
// PARTICULAR PURPOSE. See the GNU General Public        //
// License for more details.                             //
//                                                       //
// You should have received a copy of the GNU General    //
// Public License along with this program; if not,       //
// write to the Free Software Foundation, Inc.,          //
// 51 Franklin Street, 5th Floor, Boston, MA 02110-1301, //
// USA.                                                  //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//    contact:    Olaf Conrad                            //
//                Institute of Geography                 //
//                University of Goettingen               //
//                Goldschmidtstr. 5                      //
//                37077 Goettingen                       //
//                Germany                                //
//                                                       //
//    e-mail:     oconrad@saga-gis.org                   //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include <wx/tooltip.h>

#include <saga_api/saga_api.h>

#include "res_controls.h"
#include "res_commands.h"
#include "res_dialogs.h"
#include "res_images.h"

#include "helper.h"
#include "dc_helper.h"

#include "active.h"

#include "wksp_data_control.h"
#include "wksp_data_manager.h"
#include "wksp_layer.h"
#include "wksp_map.h"
#include "wksp_map_layer.h"

#include "wksp_data_layers.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#define SCROLL_RATE		5

#define SCROLL_BAR_DX	wxSystemSettings::GetMetric(wxSYS_VSCROLL_X)
#define SCROLL_BAR_DY	wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y)

//---------------------------------------------------------
#define THUMBNAIL_DIST	5


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CWKSP_Data_Buttons	*g_pData_Buttons	= NULL;


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
IMPLEMENT_CLASS(CWKSP_Data_Button, wxPanel)

//---------------------------------------------------------
BEGIN_EVENT_TABLE(CWKSP_Data_Button, wxPanel)
	EVT_PAINT			(CWKSP_Data_Button::On_Paint)
	EVT_KEY_DOWN		(CWKSP_Data_Button::On_Key)
	EVT_LEFT_DOWN		(CWKSP_Data_Button::On_Mouse_LDown)
	EVT_LEFT_DCLICK		(CWKSP_Data_Button::On_Mouse_LDClick)
	EVT_RIGHT_DOWN		(CWKSP_Data_Button::On_Mouse_RDown)
END_EVENT_TABLE()

//---------------------------------------------------------
CWKSP_Data_Button::CWKSP_Data_Button(wxWindow *pParent, class CWKSP_Data_Item *pItem)
	: wxPanel(pParent, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED)
{
	m_pManager	= NULL;
	m_pItem		= pItem;
	m_pObject	= pItem->Get_Object();
	m_Title		= pItem->Get_Name();
}

//---------------------------------------------------------
CWKSP_Data_Button::CWKSP_Data_Button(wxWindow *pParent, CWKSP_Base_Manager *pItem)
	: wxPanel(pParent, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED)
{
	m_pManager	= pItem;
	m_pItem		= NULL;
	m_pObject	= NULL;
	m_Title		= pItem->Get_Name();

	wxClientDC	dc(this);
	SetSize(wxDefaultSize.GetWidth(), dc.GetTextExtent(m_Title).GetHeight() + 10);
}

//---------------------------------------------------------
void CWKSP_Data_Button::On_Paint(wxPaintEvent &event)
{
	wxPaintDC	dc(this);
	wxRect		r(GetClientRect());

	if( m_pItem && m_pItem->GetId().IsOk() && m_pItem->Get_Object() == m_pObject && g_pData->Get(m_pObject) )
	{
		if( !GetToolTip() || GetToolTip()->GetTip().Cmp(m_pItem->Get_Name()) )
		{
			SetToolTip(m_pItem->Get_Name());
		}

		wxBitmap	BMP;

		if( m_pItem->Get_Type() == WKSP_ITEM_Table )
		{
			BMP	= IMG_Get_Bitmap(ID_IMG_WKSP_TABLE, r.GetWidth() - 1);
		}
		else
		{
			BMP	= ((CWKSP_Layer *)m_pItem)->Get_Thumbnail(r.GetWidth() - 1, r.GetHeight() - 1);
		}

		dc.DrawBitmap(BMP, r.GetLeft(), r.GetTop(), true);

		if( m_pItem->is_Selected() )
		{
			dc.SetPen(wxPen(((CWKSP_Data_Buttons *)GetParent())->Get_Active_Color()));
			Draw_Edge(dc, EDGE_STYLE_SIMPLE, r);	r.Deflate(1);
			Draw_Edge(dc, EDGE_STYLE_SIMPLE, r);	r.Deflate(1);
			Draw_Edge(dc, EDGE_STYLE_SIMPLE, r);
		}
	}
	else
	{
		dc.DrawText(m_Title, 2, 2);
	}
}

//---------------------------------------------------------
void CWKSP_Data_Button::On_Key(wxKeyEvent &event)
{
	if( event.GetKeyCode() == WXK_DELETE )
	{
		wxCommandEvent	Command;

		Command.SetId(ID_CMD_WKSP_ITEM_CLOSE);

		g_pData_Ctrl->On_Command(Command);
	}
}

//---------------------------------------------------------
void CWKSP_Data_Button::On_Mouse_LDown(wxMouseEvent &event)
{
	_Select(event.ControlDown());
}

//---------------------------------------------------------
void CWKSP_Data_Button::On_Mouse_LDClick(wxMouseEvent &event)
{
	if( _Select(false) && m_pItem )
	{
		m_pItem->On_Command(ID_CMD_WKSP_ITEM_RETURN);
	}
}

//---------------------------------------------------------
void CWKSP_Data_Button::On_Mouse_RDown(wxMouseEvent &event)
{
	if( g_pData_Ctrl->Get_Selection_Count() <= 1 )
	{
		_Select(false);
	}

	wxMenu	*pMenu	= g_pData_Ctrl->Get_Context_Menu();

	if( pMenu )
	{
		GetParent()->PopupMenu(pMenu, GetParent()->ScreenToClient(ClientToScreen(event.GetPosition())));

		delete(pMenu);

		return;
	}
}

//---------------------------------------------------------
bool CWKSP_Data_Button::_Select(bool bKeepOthers)
{
	if( m_pItem && SG_Get_Data_Manager().Exists(m_pObject) )
	{
		g_pData_Ctrl->Set_Item_Selected(m_pItem, bKeepOthers);

		return( true );
	}

	m_pObject	= NULL;

	if( m_pManager )
	{
		g_pData_Ctrl->SelectChildren(m_pManager->GetId());
		g_pData_Buttons->Update_Buttons();

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
IMPLEMENT_CLASS(CWKSP_Data_Buttons, wxScrolledWindow)

//---------------------------------------------------------
BEGIN_EVENT_TABLE(CWKSP_Data_Buttons, wxScrolledWindow)
	EVT_SIZE			(CWKSP_Data_Buttons::On_Size)
	EVT_LEFT_DOWN		(CWKSP_Data_Buttons::On_Mouse_LDown)
END_EVENT_TABLE()


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CWKSP_Data_Buttons::CWKSP_Data_Buttons(wxWindow *pParent)
	: wxScrolledWindow(pParent, -1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxFULL_REPAINT_ON_RESIZE)
{
	g_pData_Buttons	= this;

	m_xScroll		= 0;
	m_yScroll		= 0;

	m_Items			= NULL;
	m_nItems		= 0;
}

//---------------------------------------------------------
CWKSP_Data_Buttons::~CWKSP_Data_Buttons(void)
{
	_Del_Items();

	g_pData_Buttons	= NULL;
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CWKSP_Data_Buttons::On_Size(wxSizeEvent &event)
{
	_Set_Positions();
}

//---------------------------------------------------------
void CWKSP_Data_Buttons::On_Mouse_LDown(wxMouseEvent &event)
{
	g_pData_Ctrl->UnselectAll();

	Refresh();
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CWKSP_Data_Buttons::Update_Buttons(void)
{
	Freeze();

	m_Size			= g_pData->Get_Parameter("THUMBNAIL_SIZE"    )->asInt  ();
	m_bCategorised	= g_pData->Get_Parameter("THUMBNAIL_CATEGORY")->asBool ();
	m_Active_Color	= g_pData->Get_Parameter("THUMBNAIL_SELCOLOR")->asColor();

	_Del_Items();
	_Add_Items(g_pData);

	Scroll(0, 0);

	_Set_Positions();

	Thaw();
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CWKSP_Data_Buttons::_Set_Positions(void)
{
	int		xSize, ySize, xPos, yPos, xAdd, yAdd, i, x, y;

	xSize	= GetClientSize().x - SCROLL_BAR_DX;

	if( xSize < m_Size + THUMBNAIL_DIST )
	{
		xSize	= m_Size + THUMBNAIL_DIST;
	}

	xPos	= 0;
	yPos	= 0;
	xAdd	= 0;
	yAdd	= 0;

	for(i=0; i<m_nItems; i++)
	{
		CWKSP_Data_Button	*pItem	= m_Items[i];

		if( pItem->is_Title() )
		{
			xPos	 = THUMBNAIL_DIST;
			yPos	+= yAdd;	if( yPos > 0 )	yPos	+= THUMBNAIL_DIST;

			CalcScrolledPosition(0, yPos, &x, &y);
			pItem->SetSize(x, y, xSize + SCROLL_BAR_DX, -1, wxSIZE_USE_EXISTING);

			yPos	+= THUMBNAIL_DIST + pItem->GetSize().y;
			yAdd	 = 0;
		}
		else
		{
			xAdd	= m_Size;

			if( xPos + xAdd >= xSize )
			{
				xPos	 = THUMBNAIL_DIST;
				yPos	+= yAdd;
				yAdd	 = THUMBNAIL_DIST + m_Size;
			}

			yAdd	= m_Size + THUMBNAIL_DIST;

			CalcScrolledPosition(xPos, yPos, &x, &y);
			pItem->SetSize(x, y, m_Size, m_Size);

			xPos	+= THUMBNAIL_DIST + xAdd;
		}
	}

	xSize	+= SCROLL_BAR_DX;
	ySize	 = SCROLL_BAR_DY + yPos + yAdd;

	if(	m_xScroll != xSize || m_yScroll != ySize )
	{
		m_xScroll	= xSize;
		m_yScroll	= ySize;

		SetScrollbars(SCROLL_RATE, SCROLL_RATE, m_xScroll / SCROLL_RATE, m_yScroll / SCROLL_RATE);
	}
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CWKSP_Data_Buttons::_Add_Items(CWKSP_Base_Item *pItem)
{
	if( pItem )
	{
		switch( pItem->Get_Type() )
		{
		default:
			return( false );

		case WKSP_ITEM_Table:
		case WKSP_ITEM_Shapes:
		case WKSP_ITEM_TIN:
		case WKSP_ITEM_PointCloud:
		case WKSP_ITEM_Grid:
			return( _Add_Item((CWKSP_Data_Item *)pItem) );

		case WKSP_ITEM_Table_Manager:
		case WKSP_ITEM_Shapes_Type:
		case WKSP_ITEM_TIN_Manager:
		case WKSP_ITEM_PointCloud_Manager:
		case WKSP_ITEM_Grid_System:
			if( m_bCategorised )
			{
				_Add_Item((CWKSP_Base_Manager *)pItem);
			}
			break;

		case WKSP_ITEM_Data_Manager:
		case WKSP_ITEM_Grid_Manager:
		case WKSP_ITEM_Shapes_Manager:
			break;
		}

		for(int i=0; i<((CWKSP_Base_Manager *)pItem)->Get_Count(); i++)
		{
			_Add_Items(((CWKSP_Base_Manager *)pItem)->Get_Item(i));
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CWKSP_Data_Buttons::_Add_Item(CWKSP_Data_Item *pItem)
{
	if( pItem )
	{
		m_Items	= (CWKSP_Data_Button **)SG_Realloc(m_Items, (m_nItems + 1) * sizeof(CWKSP_Data_Button *));
		m_Items[m_nItems++]	= new CWKSP_Data_Button(this, pItem);

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CWKSP_Data_Buttons::_Add_Item(CWKSP_Base_Manager *pItem)
{
	if( pItem )
	{
		m_Items	= (CWKSP_Data_Button **)SG_Realloc(m_Items, (m_nItems + 1) * sizeof(CWKSP_Data_Button *));
		m_Items[m_nItems++]	= new CWKSP_Data_Button(this, pItem);

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CWKSP_Data_Buttons::_Del_Items(void)
{
	if( m_nItems > 0 )
	{
		for(int i=0; i<m_nItems; i++)
		{
			delete(m_Items[i]);
		}

		SG_Free(m_Items);
	}

	m_Items		= NULL;
	m_nItems	= 0;

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
