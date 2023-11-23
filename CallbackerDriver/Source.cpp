#pragma warning(disable: 4996)
#include <ntifs.h>
#include <ntddk.h>
#include <aux_klib.h>
#include "IOCTLs.h"
#include "Common.h"

void DriverCleanup(PDRIVER_OBJECT DriverObject);
NTSTATUS CreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS DeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
 
UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\KCallBacker");
UNICODE_STRING symlink = RTL_CONSTANT_STRING(L"\\??\\KCallBacker");

void OnProcessNotify(
	PEPROCESS Process,
	HANDLE ProcessId,
	PPS_CREATE_NOTIFY_INFO CreateInfo
);

extern "C"
NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	DbgPrint(("[+] DriverEntry called!\n"));
	DriverObject->DriverUnload = DriverCleanup;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CreateClose;

	PDEVICE_OBJECT deviceObject;

	NTSTATUS status = IoCreateDevice(
		DriverObject,
		0,
		&deviceName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&deviceObject
	);

	if (!NT_SUCCESS(status))
	{
		DbgPrint("[!] Failed to create Device Object (0x%08X)\n", status);
		return status;
	}

	status = IoCreateSymbolicLink(&symlink, &deviceName);

	if (!NT_SUCCESS(status))
	{
		DbgPrint("[!] Failed to create symlink (0x%08X)\n", status);
		IoDeleteDevice(deviceObject);
		return status;
	}


	status = PsSetCreateProcessNotifyRoutineEx(OnProcessNotify, FALSE);

	if (!NT_SUCCESS(status))
	{
		DbgPrint("PsSetCreateProcessNotifyRoutine failed (0x%08X)\n", status);
		PsSetCreateProcessNotifyRoutineEx(OnProcessNotify, TRUE);
	}

	return STATUS_SUCCESS;
}



void
OnProcessNotify(
	PEPROCESS process, HANDLE pid, PPS_CREATE_NOTIFY_INFO createInfo)
{
	UNREFERENCED_PARAMETER(process);
	UNREFERENCED_PARAMETER(pid);

	if (createInfo != NULL)
	{
		DbgPrint("[+] Process: %ws started\n", createInfo->CommandLine->Buffer);
		if (wcsstr(createInfo->CommandLine->Buffer, L"Cheat Engine") != NULL)
		{
			DbgPrint("[!] Access to launch notepad.exe was denied!");
			createInfo->CreationStatus = STATUS_ACCESS_DENIED;
		}
	}
}


NTSTATUS DeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	NTSTATUS status = STATUS_SUCCESS;
	ULONG_PTR length = 0;

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

	switch (stack->Parameters.DeviceIoControl.IoControlCode)
	{
	case KCALLBACKER_IOCTL_INIT:
	{
		break;
	}
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		DbgPrint("[!] STATUS_INVALID_DEVICE_REQUEST\n");
		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = length;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}


NTSTATUS CreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	DbgPrint("[+] Driver::KCallBacker CreateClose called\n");
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}


void DriverCleanup(PDRIVER_OBJECT DriverObject)
{
	DbgPrint("[+] Driver::KCallBacker DriverCleanup called\n");
	IoDeleteSymbolicLink(&symlink);
	IoDeleteDevice(DriverObject->DeviceObject);
	PsSetCreateProcessNotifyRoutineEx(OnProcessNotify, TRUE);
}