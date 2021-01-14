#include <iostream>
#include <string>
#include <windows.h>

using namespace std;

class Fname
{
public:
	Fname();
	Fname(const wchar_t* ws);
	Fname(const Fname& other);
	~Fname();
	
	operator const wchar_t*() const;
	Fname& operator+(wchar_t c);
	Fname& operator+(const Fname& other);
	Fname& operator=(const Fname& other);
	
	int Size() const;
	Fname& StripName(bool name = true);
	Fname& StripPath();
	Fname& PopBack();
	Fname& PopFront(int n = 1);
	Fname& PopLastName();
	
private:
	int StrSize(const wchar_t* s) const;
	void Init(const wchar_t* ws);
	
	// Data
	wchar_t* str_;
};

Fname file_name;
unsigned long long crt_time;
int counter;

void printErrStr(DWORD id);

int main()
{
	HMODULE h_mod = GetModuleHandleW(NULL);
	
	wchar_t* path = new wchar_t[MAX_PATH];
	GetModuleFileNameW(h_mod, path, MAX_PATH);
	
	Fname base = path;
	Fname tmp = L"\\\\?\\";
	base = tmp + base;	
	
	
	delete[] path;
	
	
	HANDLE file;
	_WIN32_FIND_DATAW fdata;
	
	Fname name = base;
	name.StripName();
	name = name + L'\\' + L'*';
	
	wcout << L"Renamed file #:";
	
	while(true)
	{
		Fname local_name;
		unsigned long long local_time = 0;
		
		file = FindFirstFileW(name, &fdata);
		
		do
		{
			if(file == INVALID_HANDLE_VALUE)
			{
				wcout << endl << endl << L"FindFirstFileW() Failed! Error# " << GetLastError() << endl;
				printErrStr(GetLastError());
				wcout << endl;
				return 1;
			}
			
			if(fdata.dwFileAttributes & 0x10) // Is it a folder?
			{
				continue;
			}
			
			// Ignore "." and ".." hidden super secret system sneaky folders
			const wchar_t* fn = fdata.cFileName;
			if((fn[0] == L'.' && fn[1] == 0) || (fn[1] == L'.' && fn[2] == 0))
			{
				continue;
			}
			
			// Test if it is not this probram's own file!
			Fname test = fdata.cFileName;
			test.StripPath();
			const wchar_t* b = test;
			if(b[0] == L'n' && b[1] == L'f' && b[2] == L'a' && b[3] == L'i' && b[4] == L'n' && b[5] == L'c')
			{
				continue;
			}
			
			//FILETIME ftime = fdata.ftCreationTime;
			FILETIME ftime = fdata.ftLastWriteTime;			
			const unsigned long long time = (unsigned long long)ftime.dwHighDateTime << 32 | (unsigned long long)ftime.dwLowDateTime;
			
			if(time > local_time)
			{
				local_time = time;
				local_name = fdata.cFileName;
			}
			
		}
		while(FindNextFileW(file, &fdata));
		
		if(local_time > crt_time)
		{			
			string count = to_string(counter);
			const size_t cSize = strlen(count.c_str())+1;
			wchar_t* wc = new wchar_t[cSize];
			mbstowcs (wc, count.c_str(), cSize);
			
			Fname prefix = wc;
			delete[] wc;

			Fname new_name = local_name;
			new_name = prefix + L'_' + new_name;
			
			if(!MoveFileW(local_name, new_name))
			{
				wcout << endl << endl << L"MoveFileW() Failed! Error# " << GetLastError() << endl;
				printErrStr(GetLastError());
				wcout << endl;
				Sleep(500);
				continue;
			}
			
			crt_time = local_time;
			file_name = local_name;

			wcout << L"..." << ++counter;
		}
		
		Sleep(500);
	}
	
	FindClose(file);
	
	return 0;
}

// Fname's Public Functions
Fname::Fname() : str_(nullptr)
{
	str_ = new wchar_t[1];
	str_[0] = 0;
}

Fname::Fname(const wchar_t* ws) : str_(nullptr)
{
	Init(ws);
}

Fname::Fname(const Fname& other)
{
	if(this != &other)
	{
		Init(other.str_);
	}
}

inline Fname::~Fname()
{
	delete[] str_;
}

inline Fname::operator const wchar_t*() const
{
	return str_;
}

Fname& Fname::operator+(wchar_t c)
{
	int ns = Size()+2;
	wchar_t* tmp = new wchar_t[ns];
	if(str_ != nullptr)
	{
		memcpy(tmp, str_, (ns-2) * sizeof(wchar_t));
	}
	tmp[ns-2] = c;
	tmp[ns-1] = 0;
	delete[] str_;
	str_ = tmp;
	
	return *this;
}

Fname& Fname::operator+(const Fname& other)
{
	const int s1 = Size();
	const int s2 = other.Size();
	
	wchar_t* tmp = new wchar_t[s1+s2+1];
	
	memcpy(tmp, str_, s1 * sizeof(wchar_t));
	memcpy(tmp+s1, other.str_, (s2+1) * sizeof(wchar_t));
	
	delete[] str_;
	str_ = tmp;
	
	return *this;
}

Fname& Fname::operator=(const Fname& other)
{
	if(this != &other)
	{
		int s = other.Size()+1;
		wchar_t* tmp = new wchar_t[s];
		memcpy(tmp, other.str_, s * sizeof(wchar_t));
		delete[] str_;
		str_ = tmp;
	}
	
	return *this;
}

inline int Fname::Size() const
{
	return StrSize(str_);
}

Fname& Fname::StripName(bool name)
{
	if(str_ == nullptr)
	{
		return *this;
	}
	
	const int s = Size()+1;
	
	int slash = -1;
	for(int i = 0; i < s; ++i)
	{
		slash = str_[i] == '\\' ? i : slash;
	}
	
	if(name)
	{
		// Stripping slash too...
		wchar_t* tmp = new wchar_t[slash+1];
		memcpy(tmp, str_, slash * sizeof(wchar_t));
		tmp[slash] = 0;
		delete[] str_;
		str_ = tmp;
	}
	else
	{
		const int ns = s-2-slash;
		wchar_t* tmp = new wchar_t[ns+1];
		memcpy(tmp, str_+slash+1, (ns) * sizeof(wchar_t));
		tmp[ns] = 0;
		delete[] str_;
		str_ = tmp;
	}
	
	return *this;
}

Fname& Fname::StripPath()
{
	StripName(false);
	return *this;
}

Fname& Fname::PopBack()
{
	int s = Size();
	if(s > 0)
	{
		str_[s-1] = 0;
	}
	
	return *this;
}

Fname& Fname::PopFront(int n)
{
	int s = Size() - n;
	wchar_t* tmp = new wchar_t[s+1];
	memcpy(tmp, str_+n, s * sizeof(wchar_t));
	tmp[s] = 0;
	delete[] str_;
	str_ = tmp;
	
	return *this;
}

Fname& Fname::PopLastName()
{
	for(int i = Size(); i >= 0; --i)
	{
		if(str_[i] == L'\\')
		{
			wchar_t* tmp = new wchar_t[i+1];
			memcpy(tmp, str_, i * sizeof(wchar_t));
			tmp[i] = 0;
			delete[] str_;
			str_ = tmp;
			break;
		}
	}
	
	return *this;
}

// Fname's Private Functions
int Fname::StrSize(const wchar_t* s) const
{
	if(s == nullptr)
	{
		return 0;
	}
	
	int i = 0;
	
	while(s[i])
	{
		++i;
	}
	
	return i;
}

void Fname::Init(const wchar_t* ws)
{
	int s = StrSize(ws);
	str_ = new wchar_t[s+1];
	memcpy(str_, ws, s * sizeof(wchar_t));
	str_[s] = 0;
}

void printErrStr(DWORD id)
{    
    LPSTR buff = nullptr;

    // Ask Win32 to give us the string version of that message ID.
    // The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
								NULL, id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buff, 0, NULL);
								
    wcout << buff;
    
    // Free the Win32's string's buffer.
    LocalFree(buff);
}