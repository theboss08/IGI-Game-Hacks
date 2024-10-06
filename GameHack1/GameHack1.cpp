// GameHack1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include "Proc.h"
#include "mem.h"

static void triggerBot(uintptr_t moduleBase, HANDLE hProcess);

int main() {
	// Get procId of the game process
	DWORD procId = GetProcId(L"Project IGI.exe");
	if (!procId) {
		std::cout << "Process Not Found\n";
		getchar();
		return 0;
	}

	// Get ModuleBaseAddress
	uintptr_t moduleBase = GetModuleBaseAddress(procId, L"Project IGI.exe");
	std::cout << "module base address " << moduleBase << std::endl;

	// Get Handle to the process
	HANDLE hProcess = 0;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
	if (!hProcess) {
		std::cout << "Handle to the process not found\n";
		getchar();
		return 0;
	}

	// basicHacks(moduleBase, hProcess);

	triggerBot(moduleBase, hProcess);

	getchar();
	return 0;
}

static void triggerBot(uintptr_t moduleBase, HANDLE hProcess) {
	DWORD dwExit = 0;
	while (GetExitCodeProcess(hProcess, &dwExit) && dwExit == STILL_ACTIVE) {
		// print all the position of enemy
		uintptr_t entityList = moduleBase + 0x0017D848;
		for (int i = 0; i < 4; i++) {
			uintptr_t entityPointer = 0;
			ReadProcessMemory(hProcess, (BYTE*)(entityList + 0x4 * i), &entityPointer, sizeof(entityPointer), nullptr);
			if (entityPointer == 0) continue;
			uintptr_t enemyXAddr = FindDMAAddy(hProcess, entityList + 0x4 * i, { 0x24 });
			uintptr_t enemyYAddr = enemyXAddr + 0x8;
			uintptr_t enemyZAddr = enemyYAddr + 0x8;
			if (enemyXAddr != 0 && enemyYAddr != 0 && enemyZAddr != 0) {
				float enemyX = 0, enemyY = 0, enemyZ = 0;
				ReadProcessMemory(hProcess, (BYTE*)enemyXAddr, &enemyX, sizeof(enemyX), nullptr);
				ReadProcessMemory(hProcess, (BYTE*)enemyYAddr, &enemyY, sizeof(enemyY), nullptr);
				ReadProcessMemory(hProcess, (BYTE*)enemyZAddr, &enemyY, sizeof(enemyZ), nullptr);

				std::cout << "Enemy " << i + 1 << " " << enemyX << " " << enemyY << " " << enemyZ << std::endl;
			}

			float enemyHealth = 0;
			uintptr_t enemyHealthAddr = FindDMAAddy(hProcess, entityList + 0x4 * i, { 0x254 });
			if (enemyHealthAddr != 0) {
				ReadProcessMemory(hProcess, (BYTE*)enemyHealthAddr, &enemyHealth, sizeof(enemyHealth), nullptr);
				std::cout << "Enemy health " << i + 1 << " " << enemyHealth << std::endl;
			}
		}

		Sleep(1000);
	}
}

static void basicHacks(uintptr_t moduleBase, HANDLE hProcess) {
	// Resolve our position pointer chain
	uintptr_t dynamicPositionPtr = moduleBase + 0x16E210;
	std::cout << "dynamic ptr base address " << dynamicPositionPtr << std::endl;

	DWORD dwExit = 0;
	bool bHealth = false, bAmmo = false, bFly = false;
	float z_value = 0, x_value = 0, y_value = 0;
	std::vector<unsigned int> z_offsets = { 0x8, 0x34 };
	std::vector<unsigned int> x_offsets = { 0x8, 0x2C };
	std::vector<unsigned int> y_offsets = { 0x8, 0x24 };
	uintptr_t y_addr = FindDMAAddy(hProcess, dynamicPositionPtr, y_offsets);
	uintptr_t x_addr = y_addr + 0x8;
	uintptr_t z_addr = x_addr + 0x8;

	while (GetExitCodeProcess(hProcess, &dwExit) && dwExit == STILL_ACTIVE) {

		if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
			bHealth = !bHealth;

			if (bHealth) {
				mem::NopEx((BYTE*)(moduleBase + 0x864C), 8, hProcess);
				mem::NopEx((BYTE*)(moduleBase + 0x124FF), 9, hProcess);
				mem::NopEx((BYTE*)(moduleBase + 0x12C2F), 10, hProcess);
				mem::NopEx((BYTE*)(moduleBase + 0x12E50), 10, hProcess);
			}
			else {

			}
		}
		if (GetAsyncKeyState(VK_NUMPAD2) & 1) {
			bAmmo = !bAmmo;
			if (bAmmo) {
				mem::PatchEx((BYTE*)(moduleBase + 0x7968E), (BYTE*)"\x40", 1, hProcess);
			}
			else {
				mem::PatchEx((BYTE*)(moduleBase + 0x7968E), (BYTE*)"\x48", 1, hProcess);
			}
		}
		if (GetAsyncKeyState(VK_NUMPAD3) & 1) {
			// save current position
			ReadProcessMemory(hProcess, (BYTE*)z_addr, &z_value, sizeof(z_value), nullptr);
			ReadProcessMemory(hProcess, (BYTE*)x_addr, &x_value, sizeof(x_value), nullptr);
			ReadProcessMemory(hProcess, (BYTE*)y_addr, &y_value, sizeof(y_value), nullptr);
		}
		if (GetAsyncKeyState(VK_NUMPAD4) & 1) {
			// teleport to saved position
			mem::PatchEx((BYTE*)z_addr, (BYTE*)&z_value, sizeof(z_value), hProcess);
			mem::PatchEx((BYTE*)x_addr, (BYTE*)&x_value, sizeof(x_value), hProcess);
			mem::PatchEx((BYTE*)y_addr, (BYTE*)&y_value, sizeof(y_value), hProcess);
		}
		if (GetAsyncKeyState(VK_NUMPAD5) & 1) {
			bFly = !bFly;
			if (bFly) {
				mem::NopEx((BYTE*)(moduleBase + 0xc6a1e), 3, hProcess);
				mem::NopEx((BYTE*)(moduleBase + 0xc6a2a), 3, hProcess);
				mem::NopEx((BYTE*)(moduleBase + 0xc6a36), 3, hProcess);
			}
			else {
				mem::PatchEx((BYTE*)(moduleBase + 0xc6a1e), (BYTE*)"\x89\x57\x24", 3, hProcess);
				mem::PatchEx((BYTE*)(moduleBase + 0xc6a2a), (BYTE*)"\x89\x57\x2C", 3, hProcess);
				mem::PatchEx((BYTE*)(moduleBase + 0xc6a36), (BYTE*)"\x89\x57\x34", 3, hProcess);
			}
		}
		if (GetAsyncKeyState(VK_INSERT) & 1) {
			return;
		}

		uintptr_t y_addr = FindDMAAddy(hProcess, dynamicPositionPtr, y_offsets);
		uintptr_t x_addr = y_addr + 0x8;
		uintptr_t z_addr = x_addr + 0x8;

		if ((GetKeyState('W') & 0x8000) && bFly) {
			std::cout << "w key is pressed" << std::endl;
			ReadProcessMemory(hProcess, (BYTE*)x_addr, &x_value, sizeof(x_value), nullptr);
			x_value += (float)0.0001;
			mem::PatchEx((BYTE*)x_addr, (BYTE*)&x_value, sizeof(x_value), hProcess);
		}
		if ((GetKeyState('S') & 0x8000) && bFly) {
			std::cout << "s key is pressed" << std::endl;
			ReadProcessMemory(hProcess, (BYTE*)x_addr, &x_value, sizeof(x_value), nullptr);
			x_value -= (float)0.0001;
			mem::PatchEx((BYTE*)x_addr, (BYTE*)&x_value, sizeof(x_value), hProcess);
		}

		if ((GetKeyState('A') & 0x8000) && bFly) {
			std::cout << "a key is pressed" << std::endl;
			ReadProcessMemory(hProcess, (BYTE*)y_addr, &y_value, sizeof(y_value), nullptr);
			y_value += (float)0.0001;
			mem::PatchEx((BYTE*)y_addr, (BYTE*)&y_value, sizeof(y_value), hProcess);
		}
		if ((GetKeyState('D') & 0x8000) && bFly) {
			std::cout << "d key is pressed" << std::endl;
			ReadProcessMemory(hProcess, (BYTE*)y_addr, &y_value, sizeof(y_value), nullptr);
			y_value -= (float)0.0001;
			mem::PatchEx((BYTE*)y_addr, (BYTE*)&y_value, sizeof(y_value), hProcess);
		}

		if ((GetKeyState(VK_UP) & 0x8000) && bFly) {
			std::cout << "up key is pressed" << std::endl;
			ReadProcessMemory(hProcess, (BYTE*)z_addr, &z_value, sizeof(z_value), nullptr);
			z_value += (float)0.0001;
			mem::PatchEx((BYTE*)z_addr, (BYTE*)&z_value, sizeof(z_value), hProcess);
		}
		if ((GetKeyState(VK_DOWN) & 0x8000) && bFly) {
			std::cout << "down key is pressed" << std::endl;
			ReadProcessMemory(hProcess, (BYTE*)z_addr, &z_value, sizeof(z_value), nullptr);
			z_value -= (float)0.0001;
			mem::PatchEx((BYTE*)z_addr, (BYTE*)&z_value, sizeof(z_value), hProcess);
		}

		Sleep(50);
	}
}