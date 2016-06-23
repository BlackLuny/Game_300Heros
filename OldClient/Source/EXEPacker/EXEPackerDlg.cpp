
// EXEPackerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "EXEPacker.h"
#include "EXEPackerDlg.h"
#include "afxdialogex.h"
#include <Winnt.h>
#include <string>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*rva �� raw ��ת��*/

DWORD rva2raw(WORD nSections, PIMAGE_SECTION_HEADER pSectionHeader, DWORD rva)
{
	for(int i = nSections-1; i >= 0; i--)
	{
		if (pSectionHeader[i].VirtualAddress <= rva)
		{
			return pSectionHeader[i].PointerToRawData + rva - pSectionHeader[i].VirtualAddress;
		}
	}
	return 0;
}

/*raw �� rva ��ת��*/
DWORD raw2rva(WORD nSections, PIMAGE_SECTION_HEADER pSectionHeader, DWORD raw)
{
	for(int i = nSections-1; i >= 0; i--)
	{
		if (pSectionHeader[i].PointerToRawData <= raw)
		{
			return pSectionHeader[i].VirtualAddress + raw - pSectionHeader[i].PointerToRawData;
		}
	}
	return 0;
}


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CEXEPackerDlg �Ի���




CEXEPackerDlg::CEXEPackerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEXEPackerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEXEPackerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEXEPackerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CEXEPackerDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CEXEPackerDlg ��Ϣ�������

BOOL CEXEPackerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CEXEPackerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CEXEPackerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CEXEPackerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

enum import_type							//2��import��ʽ�����ƻ���Hint
{
	by_name,
	by_ord
};

struct import_entry							//IAT��
{
	enum import_type type;
	char* pDll;								//dll��
	char* pName;							//������
	DWORD Ordinal;							//Hint
	DWORD address;							//trunk��ַ
};
#define TRUNC(a, b) (a + (b - ((a % b) ? (a % b) : b)))

VOID MakeLoadDllStubBegin(BYTE*& pbLoadCode,DWORD& LoadCodeLen,DWORD RemoteDllName,DWORD LoadLibAddr)
{
	/*
		push    edi
		mov     eax, 0x0
		mov     edi, 0x0
		push    eax
		call    edi
		mov     edi, eax
	*/
	BYTE bOpCode[] = {0x57,0xB8,0x00,0x00,0x00,0x00,0xBF,0x00,0x00,0x00,0x00,0x8B,0x3F,0x50,0xFF,0xD7,0x8B,0xF8};

	


	*(DWORD*)&bOpCode[2] = RemoteDllName;
	*(DWORD*)&bOpCode[7] = LoadLibAddr;

	DWORD NewLen = LoadCodeLen + sizeof(bOpCode);
	pbLoadCode = (BYTE*)realloc(pbLoadCode,NewLen);

	memcpy(&pbLoadCode[LoadCodeLen],&bOpCode,sizeof(bOpCode));

	LoadCodeLen = NewLen;
}

VOID MakeLoadDllStubProcess(BYTE*& pbLoadCode,DWORD& LoadCodeLen,DWORD RemoteFuncName,DWORD WritePtr,DWORD GetProcAddr)
{
	/*
		mov     eax, 0x0
		push    eax
		push    edi
		mov     eax, 0x0
		call    eax
		mov     ecx, 0x0
		mov     dword ptr [ecx], eax
	*/
	BYTE bOpCode[] = {0xB8,0x00,0x00,0x00,0x00,0x50,0x57,0xB8,0x00,0x00,0x00,0x00,0x8B,0x00,0xFF,0xD0,0xB9,0x00,0x00,0x00,0x00,0x89,0x01};
	//���ú�����
	*(DWORD*)&bOpCode[1] = RemoteFuncName;
	*(DWORD*)&bOpCode[8] = (DWORD)GetProcAddr;
	*(DWORD*)&bOpCode[17] = WritePtr;

	DWORD NewLen = LoadCodeLen + sizeof(bOpCode);
	pbLoadCode = (BYTE*)realloc(pbLoadCode,NewLen);

	memcpy(&pbLoadCode[LoadCodeLen],&bOpCode,sizeof(bOpCode));

	LoadCodeLen = NewLen;

}

VOID MakeLoadDllStubEnd(BYTE*& pbLoadCode,DWORD& LoadCodeLen)
{
	/*
		pop		edi
	*/
	BYTE bOpCode[] = {0x5F};

	DWORD NewLen = LoadCodeLen + sizeof(bOpCode);
	pbLoadCode = (BYTE*)realloc(pbLoadCode,NewLen);

	memcpy(&pbLoadCode[LoadCodeLen],&bOpCode,sizeof(bOpCode));

	LoadCodeLen = NewLen;
}

VOID MakeRunDllCodeLocal(BYTE*& pbLoadCode,DWORD& LoadCodeLen,DWORD hInst,DWORD EntryPoint,DWORD oldEntry)
{
	/*
		push    0x0
		push    0x1
		mov     eax, 0x0	//hInst
		push    eax
		mov     eax, 0x0	//EntryPoint
		call    eax
		push	0
		retn
	*/

	BYTE bOpCode[] = {0x6A,0x00,0x6A,0x01,0xB8,0x00,0x00,0x00,0x00,0x50,0xB8,0x00,0x00,0x00,0x00,0xFF,0xD0,0xB8,0,0,0,0,0xFF,0xD0,0xC3};

	*(DWORD*)&bOpCode[5] = hInst;
	*(DWORD*)&bOpCode[11] = EntryPoint;
	*(DWORD*)&bOpCode[18] = oldEntry;
	
	DWORD NewLen = LoadCodeLen + sizeof(bOpCode);
	pbLoadCode = (BYTE*)realloc(pbLoadCode,NewLen);

	memcpy(&pbLoadCode[LoadCodeLen],&bOpCode,sizeof(bOpCode));

	LoadCodeLen = NewLen;
}


void CEXEPackerDlg::OnBnClickedButton1()
{
	CFile MainFile;
	CFile DllFile;

	if(MainFile.Open("C:\\data\\games\\300Ӣ�۾ɰ�ͻ���P16\\300Hero.exe",CFile::modeRead) == FALSE)
	{
		AfxMessageBox("���ļ�ʧ��!");
		return;
	}

	if(DllFile.Open("C:\\data\\games\\300Ӣ�۾ɰ�ͻ���P16\\GamePatch.dll",CFile::modeRead) == FALSE)
	{
		AfxMessageBox("���ļ�ʧ��!");
		return;
	}

	DWORD EXESize = (DWORD)MainFile.GetLength();
	BYTE* pEXEBuf = (BYTE*)VirtualAlloc(NULL,EXESize,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	if(MainFile.Read(pEXEBuf,EXESize) == FALSE)
	{
		AfxMessageBox("EXE�ļ���ȡʧ��");
		return;
	}

	DWORD DllSize = (DWORD)DllFile.GetLength();
	BYTE* pDllBuf = (BYTE*)VirtualAlloc(NULL,DllSize,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	if(DllFile.Read(pDllBuf,DllSize) == FALSE)
	{
		AfxMessageBox("EXE�ļ���ȡʧ��");
		return;
	}
	DWORD SizeOfExe = (DWORD)MainFile.GetLength();
	PIMAGE_DOS_HEADER DOSHeader_EXE = (PIMAGE_DOS_HEADER)pEXEBuf;
	PIMAGE_NT_HEADERS NTHeaders_EXE = (PIMAGE_NT_HEADERS)&pEXEBuf[DOSHeader_EXE->e_lfanew];
	PIMAGE_SECTION_HEADER SectionHeader_EXE = IMAGE_FIRST_SECTION(NTHeaders_EXE);

	PIMAGE_DOS_HEADER DOSHeader_Dll = (PIMAGE_DOS_HEADER)pDllBuf;
	PIMAGE_NT_HEADERS NTHeaders_Dll = (PIMAGE_NT_HEADERS)&pDllBuf[DOSHeader_Dll->e_lfanew];
	PIMAGE_SECTION_HEADER SectionHeader_Dll = IMAGE_FIRST_SECTION(NTHeaders_Dll);

	DWORD AppendVA = 0;

	for(WORD i=0;i<NTHeaders_EXE->FileHeader.NumberOfSections;i++)
	{
		AppendVA += TRUNC(SectionHeader_EXE[i].Misc.VirtualSize,NTHeaders_EXE->OptionalHeader.SectionAlignment);
	}
	/*
		չ��DLL����
	*/

	BYTE* DllExtract = (BYTE *)VirtualAlloc(NULL,NTHeaders_Dll->OptionalHeader.SizeOfImage,MEM_COMMIT,PAGE_EXECUTE_READWRITE);


	for(WORD i=0;i<NTHeaders_Dll->FileHeader.NumberOfSections;i++)
	{
		BYTE *SrcData = &pDllBuf[SectionHeader_Dll[i].PointerToRawData];
		memcpy(&DllExtract[SectionHeader_Dll[i].VirtualAddress],SrcData,SectionHeader_Dll[i].SizeOfRawData);
	}

	DWORD dwEntryPoint = 0;

	
	PIMAGE_DATA_DIRECTORY pRelocDir = &NTHeaders_Dll->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	PIMAGE_BASE_RELOCATION pBaseReloc = (PIMAGE_BASE_RELOCATION)&DllExtract[pRelocDir->VirtualAddress];

	do 
	{
		DWORD dwRelocCount = (pBaseReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
		PWORD pRelocItem = (PWORD)((BYTE*)pBaseReloc + sizeof(IMAGE_BASE_RELOCATION));

		for(DWORD i=0;i<dwRelocCount;i++)
		{
			BYTE bType = (BYTE)(pRelocItem[i] >> 12);
			DWORD dwRelocOffset = (DWORD)(pRelocItem[i] & 0x0FFF);

			if(bType == IMAGE_REL_BASED_HIGHLOW)
			{
				DWORD* RelocAddrPtr = (DWORD*)((DWORD)DllExtract + dwRelocOffset + pBaseReloc->VirtualAddress);

				DWORD dwFixPtr =  *RelocAddrPtr - NTHeaders_Dll->OptionalHeader.ImageBase + AppendVA + NTHeaders_EXE->OptionalHeader.ImageBase;

				*RelocAddrPtr = dwFixPtr;
			}
		}

		pBaseReloc = (PIMAGE_BASE_RELOCATION)((BYTE*)pBaseReloc + pBaseReloc->SizeOfBlock);		
	} while (pBaseReloc->VirtualAddress != NULL);

	//����ض�λ��������յ�

	PIMAGE_SECTION_HEADER pRelocSectionHeader = &SectionHeader_Dll[NTHeaders_Dll->FileHeader.NumberOfSections - 1]; //Last Section
	char* reloc_section_name = ".reloc";
	if(!memcmp(pRelocSectionHeader->Name, reloc_section_name, sizeof(reloc_section_name)))  //if (Last Section.Name == ".reloc") ?
	{
		NTHeaders_Dll->OptionalHeader.SizeOfImage -= TRUNC(pRelocSectionHeader->Misc.VirtualSize, NTHeaders_Dll->OptionalHeader.SectionAlignment); //����ȥ��.reloc���ImageSize //TRUNC() ���ж���
		NTHeaders_Dll->FileHeader.NumberOfSections--;    //����������
	}

	PIMAGE_DATA_DIRECTORY pImportDir = &NTHeaders_Dll->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_IMPORT_DESCRIPTOR pImpEntry = (PIMAGE_IMPORT_DESCRIPTOR)&DllExtract[pImportDir->VirtualAddress];

	DWORD DllAppendData = 0;

	for(WORD i=0;i<NTHeaders_Dll->FileHeader.NumberOfSections;i++)
	{
		DllAppendData += SectionHeader_Dll[i].SizeOfRawData;
	}
	

	BYTE* pNewExeBuf = (BYTE*)malloc(SizeOfExe + DllAppendData);

	memcpy(pNewExeBuf,pEXEBuf,SizeOfExe);

	PIMAGE_DOS_HEADER DOSHeader_New = (PIMAGE_DOS_HEADER)pNewExeBuf;
	PIMAGE_NT_HEADERS NTHeaders_New = (PIMAGE_NT_HEADERS)&pNewExeBuf[DOSHeader_New->e_lfanew];
	PIMAGE_SECTION_HEADER SectionHeader_New = IMAGE_FIRST_SECTION(NTHeaders_New);

	DWORD EndOfOldData = NTHeaders_New->OptionalHeader.SizeOfHeaders;

	for(WORD i=0;i<NTHeaders_New->FileHeader.NumberOfSections;i++)
	{
		EndOfOldData += SectionHeader_New[i].SizeOfRawData;
	}

	for(WORD i=0;i<NTHeaders_Dll->FileHeader.NumberOfSections;i++)
	{
		SectionHeader_New[NTHeaders_New->FileHeader.NumberOfSections + i] = SectionHeader_Dll[i];
		if(stricmp((char*)&SectionHeader_New[NTHeaders_New->FileHeader.NumberOfSections + i].Name,".rdata")==0)
		{
			SectionHeader_New[NTHeaders_New->FileHeader.NumberOfSections + i].Characteristics = IMAGE_SCN_CNT_INITIALIZED_DATA|IMAGE_SCN_MEM_READ|IMAGE_SCN_MEM_WRITE;
		}
	}
	
	
	for(WORD i=0;i<NTHeaders_Dll->FileHeader.NumberOfSections;i++)
	{
		memcpy(&pNewExeBuf[EndOfOldData],&DllExtract[SectionHeader_Dll[i].VirtualAddress],SectionHeader_Dll[i].SizeOfRawData);
		SectionHeader_New[NTHeaders_New->FileHeader.NumberOfSections + i].PointerToRawData = EndOfOldData;
		SectionHeader_New[NTHeaders_New->FileHeader.NumberOfSections + i].VirtualAddress += AppendVA;
		EndOfOldData += SectionHeader_Dll[i].SizeOfRawData;
	}
	DWORD NewImageSize = 0x1000;
	//TRUNC

	NTHeaders_New->FileHeader.NumberOfSections += NTHeaders_Dll->FileHeader.NumberOfSections;

	/*
		����ԭʼ�ĵ����
	*/
	//rva2raw
#define GET_EXE_RAW(_X_) rva2raw(NTHeaders_EXE->FileHeader.NumberOfSections,SectionHeader_EXE,_X_)

	PIMAGE_DATA_DIRECTORY pImportDirExe = &NTHeaders_EXE->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_IMPORT_DESCRIPTOR pImpEntryExe = (PIMAGE_IMPORT_DESCRIPTOR)&pEXEBuf[GET_EXE_RAW(pImportDirExe->VirtualAddress)];
	DWORD LoadLibAddr = 0;
	DWORD GetProcAddr = 0;
	do 
	{
		char* pDllName = (char*)&pEXEBuf[GET_EXE_RAW(pImpEntryExe->Name)];


		PIMAGE_THUNK_DATA32 pOrgImpThunk = (PIMAGE_THUNK_DATA32)&pEXEBuf[GET_EXE_RAW(pImpEntryExe->OriginalFirstThunk)];
		PIMAGE_THUNK_DATA32 pAddressThunk = (PIMAGE_THUNK_DATA32)&pEXEBuf[GET_EXE_RAW(pImpEntryExe->FirstThunk)];
		do 
		{
			if(IMAGE_SNAP_BY_ORDINAL32(pOrgImpThunk->u1.AddressOfData) == FALSE)
			{
				PIMAGE_IMPORT_BY_NAME pByName = (PIMAGE_IMPORT_BY_NAME)&pEXEBuf[GET_EXE_RAW(pOrgImpThunk->u1.AddressOfData)];
				char* pFuncName = (char*)&pByName->Name;
				if(strcmp(pFuncName,"LoadLibraryA")==0)
				{
					DWORD a = (DWORD)pAddressThunk - (DWORD)pEXEBuf;
					LoadLibAddr = raw2rva(NTHeaders_EXE->FileHeader.NumberOfSections,SectionHeader_EXE,a) + NTHeaders_EXE->OptionalHeader.ImageBase;
				}
				if(strcmp(pFuncName,"GetProcAddress")==0)
				{
					DWORD a = (DWORD)pAddressThunk - (DWORD)pEXEBuf;
					GetProcAddr = raw2rva(NTHeaders_EXE->FileHeader.NumberOfSections,SectionHeader_EXE,a) + NTHeaders_EXE->OptionalHeader.ImageBase;
				}
			}
			pOrgImpThunk++;
			pAddressThunk++;
		} while (pOrgImpThunk->u1.AddressOfData != NULL);
		pImpEntryExe++;
	} while (pImpEntryExe->Characteristics != NULL);



	BYTE* pLoadCode = (BYTE*)malloc(0);
	DWORD LoadCodeLen = 0;

	do 
	{
		char* pDllName = (char*)&DllExtract[pImpEntry->Name];
		DWORD RelocDllName = AppendVA + pImpEntry->Name + NTHeaders_EXE->OptionalHeader.ImageBase;

		PIMAGE_THUNK_DATA32 pOrgImpThunk = (PIMAGE_THUNK_DATA32)&DllExtract[pImpEntry->OriginalFirstThunk];
		PIMAGE_THUNK_DATA32 pAddressThunk = (PIMAGE_THUNK_DATA32)&DllExtract[pImpEntry->FirstThunk];
		MakeLoadDllStubBegin(pLoadCode,LoadCodeLen,RelocDllName,LoadLibAddr);
		do 
		{
			if(IMAGE_SNAP_BY_ORDINAL32(pOrgImpThunk->u1.AddressOfData))
			{
				DWORD dwOrd = IMAGE_ORDINAL(pOrgImpThunk->u1.AddressOfData);
				DWORD RelocWrtAddress = (DWORD)pAddressThunk- (DWORD)DllExtract + AppendVA + NTHeaders_EXE->OptionalHeader.ImageBase;

				MakeLoadDllStubProcess(pLoadCode,LoadCodeLen,dwOrd,RelocWrtAddress,GetProcAddr);
			}
			else
			{
				PIMAGE_IMPORT_BY_NAME pByName = (PIMAGE_IMPORT_BY_NAME)&DllExtract[pOrgImpThunk->u1.AddressOfData];
				char* pFuncName = (char*)&pByName->Name;

				DWORD RelocFuncName = (DWORD)pFuncName - (DWORD)DllExtract + AppendVA + NTHeaders_EXE->OptionalHeader.ImageBase;
				DWORD RelocWrtAddress = (DWORD)pAddressThunk- (DWORD)DllExtract + AppendVA  + NTHeaders_EXE->OptionalHeader.ImageBase;
				MakeLoadDllStubProcess(pLoadCode,LoadCodeLen,RelocFuncName,RelocWrtAddress,GetProcAddr);
			}
			pOrgImpThunk++;
			pAddressThunk++;
		} while (pOrgImpThunk->u1.AddressOfData != NULL);
		MakeLoadDllStubEnd(pLoadCode,LoadCodeLen);
		pImpEntry++;
	} while (pImpEntry->Characteristics != NULL);


	MakeRunDllCodeLocal(pLoadCode,LoadCodeLen,NTHeaders_New->OptionalHeader.ImageBase,NTHeaders_Dll->OptionalHeader.AddressOfEntryPoint + AppendVA + NTHeaders_New->OptionalHeader.ImageBase,NTHeaders_New->OptionalHeader.ImageBase+NTHeaders_New->OptionalHeader.AddressOfEntryPoint);
	NTHeaders_New->FileHeader.NumberOfSections++;
#define END_SECTION_NEW (NTHeaders_New->FileHeader.NumberOfSections - 1)
#define LAST_SECTION_NEW (NTHeaders_New->FileHeader.NumberOfSections - 2)
	SectionHeader_New[END_SECTION_NEW] = SectionHeader_New[LAST_SECTION_NEW];

	SectionHeader_New[END_SECTION_NEW].VirtualAddress =
		TRUNC(SectionHeader_New[LAST_SECTION_NEW].Misc.VirtualSize, NTHeaders_New->OptionalHeader.SectionAlignment) +
		SectionHeader_New[LAST_SECTION_NEW].VirtualAddress;

	SectionHeader_New[END_SECTION_NEW].Misc.VirtualSize = TRUNC(LoadCodeLen, NTHeaders_New->OptionalHeader.SectionAlignment);
	SectionHeader_New[END_SECTION_NEW].PointerToRawData = EndOfOldData;
	SectionHeader_New[END_SECTION_NEW].SizeOfRawData = LoadCodeLen;
	SectionHeader_New[END_SECTION_NEW].Characteristics = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_MEM_EXECUTE;

	pNewExeBuf = (BYTE*)realloc(pNewExeBuf, SizeOfExe + DllAppendData + LoadCodeLen);
	DOSHeader_New = (PIMAGE_DOS_HEADER)pNewExeBuf;
	NTHeaders_New = (PIMAGE_NT_HEADERS)&pNewExeBuf[DOSHeader_New->e_lfanew];
	SectionHeader_New = IMAGE_FIRST_SECTION(NTHeaders_New);

	memcpy(&pNewExeBuf[EndOfOldData],pLoadCode,LoadCodeLen);

	for(WORD i=0;i<NTHeaders_New->FileHeader.NumberOfSections;i++)
	{
		NewImageSize += TRUNC(SectionHeader_New[i].Misc.VirtualSize,NTHeaders_New->OptionalHeader.SectionAlignment);
		//memset(&SectionHeader_New[i].Name,0,sizeof(SectionHeader_New[i].Name));
		//memcpy(&SectionHeader_New[i].Name,"175pt",6);
	}


	NTHeaders_New->OptionalHeader.SizeOfImage = NewImageSize;
	NTHeaders_New->OptionalHeader.AddressOfEntryPoint = SectionHeader_New[NTHeaders_New->FileHeader.NumberOfSections - 1].VirtualAddress;
	CFile NewTest;
	if(NewTest.Open("C:\\data\\games\\300Ӣ�۾ɰ�ͻ���P16\\New.exe",CFile::modeCreate|CFile::modeReadWrite))
	{
		NewTest.Write(pNewExeBuf,SizeOfExe + DllAppendData + LoadCodeLen);
	}
}
