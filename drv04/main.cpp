#include <ntddk.h>
#define DRV04_BUFFER_LENGTH 16

struct MyDevice {
	UNICODE_STRING uniszDevice;
	PCWSTR pszDevice;
	UNICODE_STRING uniszSymlink;
	PCWSTR pszSymlink;
	PDEVICE_OBJECT pDeviceObject;
	ULONG uFlags;
};

MyDevice aMyDevices[3] = {0};

typedef struct _DEVICE_EXTENSION {
	unsigned char Image[DRV04_BUFFER_LENGTH];
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

VOID
OnDrvUnload(
	_In_ PDRIVER_OBJECT pDriverObject
);

NTSTATUS
DispatchDriver(
	PDEVICE_OBJECT pDeviceObject,
	PIRP pIrp
);

NTSTATUS
DispatchDriver(
	PDEVICE_OBJECT pDeviceObject,
	PIRP pIrp
)
{
	PDEVICE_EXTENSION pDeviceExtension;
	PIO_STACK_LOCATION pIoStack;
	PVOID pBuf;
	ULONG uFlags;

	KdPrint(("Func %s\n", __FUNCTION__));
	pBuf = NULL;
	pIoStack = IoGetCurrentIrpStackLocation(pIrp);
	pDeviceExtension = (PDEVICE_EXTENSION)pDeviceObject->DeviceExtension;
	uFlags = pDeviceObject->Flags & (
		DO_BUFFERED_IO |
		DO_DIRECT_IO |
		0
	);

	switch (pIoStack->MajorFunction) {
	case IRP_MJ_CREATE:
		break;
	case IRP_MJ_CLOSE:
		break;
	case IRP_MJ_READ:
	{
		ULONG uBufActualLength = min(
			DRV04_BUFFER_LENGTH,
			pIoStack->Parameters.Write.Length
		);
		switch (uFlags) {
		case DO_BUFFERED_IO:
			pBuf = pIrp->AssociatedIrp.SystemBuffer;
			break;
		case DO_DIRECT_IO:
			pBuf = MmGetSystemAddressForMdl(pIrp->MdlAddress);
			break;
		case 0:
			pBuf = pIrp->UserBuffer;
			break;
		default:
			break;
		}
		RtlMoveMemory(
			pBuf, pDeviceExtension->Image,
			uBufActualLength
		);
		pIrp->IoStatus.Information = uBufActualLength;
	}
		break;
	case IRP_MJ_WRITE:
	{
		ULONG uBufActualLength = min(
			DRV04_BUFFER_LENGTH,
			pIoStack->Parameters.Read.Length
		);
		switch (uFlags) {
		case DO_BUFFERED_IO:
			pBuf = pIrp->AssociatedIrp.SystemBuffer;
			break;
		case DO_DIRECT_IO:
			pBuf = MmGetSystemAddressForMdl(pIrp->MdlAddress);
			break;
		case 0:
			pBuf = pIrp->UserBuffer;
			break;
		default:
			break;
		}
		RtlMoveMemory(
			pDeviceExtension->Image, pBuf,
			uBufActualLength
		);
		pIrp->IoStatus.Information =
			uBufActualLength;
	}
		break;
	default:
		break;
	}
	
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	KdPrint((
		"Func %s/irp_mj_%d returns 0x%lX, information 0x%lX\n",
		__FUNCTION__, pIoStack->MajorFunction, pIrp->IoStatus.Status, pIrp->IoStatus.Information
	));
	return pIrp->IoStatus.Status;
}

VOID
OnDrvUnload(
	_In_ PDRIVER_OBJECT pDriverObject
)
{
	KdPrint(("Func %s\n", __FUNCTION__));

	for (ULONG i = 0; i < ARRAYSIZE(aMyDevices); ++i) {
		if (aMyDevices[i].uniszSymlink.Length != 0) {
			NTSTATUS nt = IoDeleteSymbolicLink(&aMyDevices[i].uniszSymlink);
			KdPrint(("Func %s/%s returns 0x%lX\n", __FUNCTION__, "IoDeleteSymbolicLink", nt));
		}
	}

	for (
		PDEVICE_OBJECT pDeviceObject = pDriverObject->DeviceObject;
		pDeviceObject != NULL;
		pDeviceObject = pDeviceObject->NextDevice
	) {
		IoDeleteDevice(pDriverObject->DeviceObject);
	}

	KdPrint(("Func %s returns\n", __FUNCTION__));
	return;
}

extern "C"
NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT pDriverObject,
	_In_ PUNICODE_STRING puniszRegistryPath
)
{
	NTSTATUS nt = STATUS_SUCCESS;
	CONST UCHAR iIrpCodes[] = {
		IRP_MJ_CREATE,
		IRP_MJ_CLOSE,
		IRP_MJ_WRITE,
		IRP_MJ_READ
	};

	UNREFERENCED_PARAMETER(puniszRegistryPath);

	KdPrint(("Func: %s\n", __FUNCTION__));

	aMyDevices[0].pszDevice = L"\\Device\\drv04-bufferio";
	aMyDevices[1].pszDevice = L"\\Device\\drv04-directio";
	aMyDevices[2].pszDevice = L"\\Device\\drv04-neitherio";
	aMyDevices[0].pszSymlink = L"\\DosDevices\\drv04-bufferio-symlink";
	aMyDevices[1].pszSymlink = L"\\DosDevices\\drv04-directio-symlink";
	aMyDevices[2].pszSymlink = L"\\DosDevices\\drv04-neitherio-symlink";
	aMyDevices[0].uFlags = DO_BUFFERED_IO;
	aMyDevices[1].uFlags = DO_DIRECT_IO;
	aMyDevices[2].uFlags = 0;

	for (ULONG i = 0; i < ARRAYSIZE(aMyDevices); ++i) {
		RtlInitUnicodeString(
			&aMyDevices[i].uniszDevice,
			aMyDevices[i].pszDevice
		);
		nt = IoCreateDevice(
			pDriverObject,
			sizeof(DEVICE_EXTENSION), &aMyDevices[i].uniszDevice,
			42069+i, aMyDevices[i].uFlags, FALSE,
			&aMyDevices[i].pDeviceObject
		);
		KdPrint(("Func %s/%s returns 0x%lX\n", __FUNCTION__, "IoCreateDevice", nt));
		if (!NT_SUCCESS(nt)) {
			OnDrvUnload(pDriverObject);
			goto fail;
		}
		RtlInitUnicodeString(
			&aMyDevices[i].uniszSymlink,
			aMyDevices[i].pszSymlink
		);
		nt = IoCreateSymbolicLink(
			&aMyDevices[i].uniszSymlink,
			&aMyDevices[i].uniszDevice
		);
		KdPrint(("Func %s/%s returns 0x%lX\n", __FUNCTION__, "IoCreateSymbolicLink", nt));
		if (!NT_SUCCESS(nt)) {
			OnDrvUnload(pDriverObject);
			goto fail;
		}
	}

	pDriverObject->DriverUnload = OnDrvUnload;
	for (ULONG i = 0; i < ARRAYSIZE(iIrpCodes); ++i) {
		pDriverObject->MajorFunction[iIrpCodes[i]] = DispatchDriver;
	}

fail:
	KdPrint(("Func %s returns 0x%lX\n", __FUNCTION__, nt));
	return nt;
}