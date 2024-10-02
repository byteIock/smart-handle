#include <iostream>
#include <Windows.h>
#include <vector>
#include <concepts>


template<typename T>
concept DefaultType = requires {
	(std::is_same_v<T, void*> || std::is_same_v<T, DWORD> || std::is_same_v<T, HANDLE>);
};

template<DefaultType T>
class SmartHandle { 
private:
	T handle = { 0 };

public:
	void releasing_handle() {
		if (this->handle != INVALID_HANDLE_VALUE && this->handle != nullptr) {
			CloseHandle(handle); handle = nullptr;
		}
	}

	void check_if_valid() {
		if (this->handle == INVALID_HANDLE_VALUE || this->handle == nullptr) {
			std::cout << "[+] invalid handle\n";
		}
	}

	explicit SmartHandle(T user_handle) : handle{ user_handle } {};
	SmartHandle(const SmartHandle&) = delete; 
	SmartHandle operator=(const SmartHandle&) = delete;
	SmartHandle(SmartHandle&& other) noexcept {
		this->handle = other.handle;
	}
	SmartHandle& operator=(SmartHandle&& other) noexcept {
		if (this != other) {
			releasing_handle();
			handle = other.handle();
			other.handle = nullptr;
		}
		return *this;
	}


	~SmartHandle() {
		releasing_handle();
	};

	T get_handle() {
		check_if_valid();
		SetHandleInformation(handle, HANDLE_FLAG_PROTECT_FROM_CLOSE, FALSE); return handle;
	}
};


void write_process_memory(_In_ std::initializer_list<void*> const list, _In_ void* handle, _In_ DWORD write_buffer) {
	for (auto const& addresses : list) {
		::WriteProcessMemory(handle, addresses, &write_buffer, sizeof(write_buffer), NULL);
	}
}

int main() {
	uintptr_t writeable_buffer = 1337;
	SmartHandle<void*> CheatHandle(OpenProcess(PROCESS_ALL_ACCESS, FALSE, 60820));
	write_process_memory({ (void*)0x006B55A4, (void*)0x006B55F8, }, CheatHandle.get_handle(), writeable_buffer);
} 
