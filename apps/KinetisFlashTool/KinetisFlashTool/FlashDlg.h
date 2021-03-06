/*
* Copyright (c) 2013-15, Freescale Semiconductor, Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* o Redistributions of source code must retain the above copyright notice, this list
*   of conditions and the following disclaimer.
*
* o Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* o Neither the name of Freescale Semiconductor, Inc. nor the names of its
*   contributors may be used to endorse or promote products derived from this
*   software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once
#include "afxwin.h"

// FlashDlg Dialog
/*!
* \brief FlashDlg class is the UI Dialog class for the
*  Flash utilities tab page
*/
class FlashDlg : public CDialogEx
{
    DECLARE_DYNAMIC(FlashDlg)

public:
    //! \brief Generated standard constructor.
    FlashDlg(CWnd *pParent = NULL); // standard constructor

    //! \brief Generated standard destructor.
    virtual ~FlashDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_FLASH
    };
#endif

protected:
    //! \brief Generated message map functions.
    virtual void DoDataExchange(CDataExchange *pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()
    virtual void OnOK();
    virtual void OnCancel();

public:
    //! \brief Generated message map functions.
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangeComboEraseOpt();
    afx_msg void OnBnClickedButtonErase();
    afx_msg void OnBnClickedButtonIfr();

    CComboBox m_comboEraseOpt; //!< Control variable for erase option combox
    CComboBox m_comboMemoryID; //!< Control variable for memory option combox
};
