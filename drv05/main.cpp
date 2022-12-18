#include <ntddk.h>
#define DRV04_BUFFER_LENGTH 16U

#define FILE_DEVICE_IOCTL 0x00008301
#define IOCTL_MY_NEITHER CTL_CODE(FILE_DEVICE_IOCTL, 0x820, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MY_BUFFERED CTL_CODE(FILE_DEVICE_IOCTL, 0x821, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MY_IN_DIRECT CTL_CODE(FILE_DEVICE_IOCTL, 0x822, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_MY_OUT_DIRECT CTL_CODE(FILE_DEVICE_IOCTL, 0x823, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

static PCWSTR pszDevice = L"\\Device\\drv04-bufferio";
UNICODE_STRING uniszDevice;
static PCWSTR pszSymlink = L"\\DosDevices\\drv04-bufferio-symlink";
UNICODE_STRING uniszSymlink;

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
	PVOID pInBuf = NULL;
	PVOID pOutBuf = NULL;
	ULONG ulInLength = 0;
	ULONG ulOutLength = 0;

	KdPrint(("Func %s\n", __FUNCTION__));
	pIoStack = IoGetCurrentIrpStackLocation(pIrp);
	pDeviceExtension = (PDEVICE_EXTENSION)pDeviceObject->DeviceExtension;

	switch (pIoStack->MajorFunction) {
	case IRP_MJ_CREATE:
		break;
	case IRP_MJ_CLOSE:
		break;
	case IRP_MJ_DEVICE_CONTROL:
		switch (pIoStack->Parameters.DeviceIoControl.IoControlCode) {
		case IOCTL_MY_BUFFERED:
			pInBuf = pIrp->AssociatedIrp.SystemBuffer;
			ulInLength = pIoStack->Parameters.DeviceIoControl.InputBufferLength;
			pOutBuf = pIrp->AssociatedIrp.SystemBuffer;
			ulOutLength = pIoStack->Parameters.DeviceIoControl.OutputBufferLength;
		case IOCTL_MY_IN_DIRECT:
			pInBuf = pIrp->AssociatedIrp.SystemBuffer;
			ulInLength = pIoStack->Parameters.DeviceIoControl.InputBufferLength;
			pOutBuf = MmGetSystemAddressForMdl(pIrp->MdlAddress);
			ulOutLength = pIoStack->Parameters.DeviceIoControl.OutputBufferLength;
			break;
		case IOCTL_MY_OUT_DIRECT:
			pInBuf = pIrp->AssociatedIrp.SystemBuffer;
			ulInLength = pIoStack->Parameters.DeviceIoControl.InputBufferLength;
			pOutBuf = MmGetSystemAddressForMdl(pIrp->MdlAddress);
			ulOutLength = pIoStack->Parameters.DeviceIoControl.OutputBufferLength;
			break;
		case IOCTL_MY_NEITHER:
			pInBuf = pIoStack->Parameters.DeviceIoControl.Type3InputBuffer;
			ulInLength = pIoStack->Parameters.DeviceIoControl.InputBufferLength;
			pOutBuf = pIrp->UserBuffer;
			ulOutLength = pIoStack->Parameters.DeviceIoControl.OutputBufferLength;
			break;
		default:
			switch (METHOD_FROM_CTL_CODE(pIoStack->Parameters.DeviceIoControl.IoControlCode)) {
			case METHOD_BUFFERED:
				pInBuf = pIrp->AssociatedIrp.SystemBuffer;
				ulInLength = pIoStack->Parameters.DeviceIoControl.InputBufferLength;
				pOutBuf = pIrp->AssociatedIrp.SystemBuffer;
				ulOutLength = pIoStack->Parameters.DeviceIoControl.OutputBufferLength;
			case METHOD_IN_DIRECT:
				pInBuf = pIrp->AssociatedIrp.SystemBuffer;
				ulInLength = pIoStack->Parameters.DeviceIoControl.InputBufferLength;
				pOutBuf = MmGetSystemAddressForMdl(pIrp->MdlAddress);
				ulOutLength = pIoStack->Parameters.DeviceIoControl.OutputBufferLength;
				break;
			case METHOD_OUT_DIRECT:
				pInBuf = pIrp->AssociatedIrp.SystemBuffer;
				ulInLength = pIoStack->Parameters.DeviceIoControl.InputBufferLength;
				pOutBuf = MmGetSystemAddressForMdl(pIrp->MdlAddress);
				ulOutLength = pIoStack->Parameters.DeviceIoControl.OutputBufferLength;
				break;
			case METHOD_NEITHER:
				pInBuf = pIoStack->Parameters.DeviceIoControl.Type3InputBuffer;
				ulInLength = pIoStack->Parameters.DeviceIoControl.InputBufferLength;
				pOutBuf = pIrp->UserBuffer;
				ulOutLength = pIoStack->Parameters.DeviceIoControl.OutputBufferLength;
				break;
			}
			break;
		}
	default:
		break;
	}

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	KdPrint((
		"drv05: InB = %08lX, len = %lX, OutB = %08lX, len = %lX\n",
		(ULONG)pInBuf, ulInLength, (ULONG)pOutBuf, ulOutLength
	));
	KdPrint((
		"Func %s/irp_mj_%d returns 0x%lX, device code 0x%lX, information 0x%lX\n",
		__FUNCTION__,
		pIoStack->MajorFunction,
		pIrp->IoStatus.Status,
		pIoStack->Parameters.DeviceIoControl.IoControlCode,
		pIrp->IoStatus.Information
	));
	return pIrp->IoStatus.Status;
}

VOID
OnDrvUnload(
	_In_ PDRIVER_OBJECT pDriverObject
)
{
	NTSTATUS nt;

	KdPrint(("Func %s\n", __FUNCTION__));
	
	nt = IoDeleteSymbolicLink(&uniszSymlink);
	KdPrint(("Func %s/%s returns 0x%lX\n", __FUNCTION__, "IoDeleteSymbolicLink", nt));

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
		IRP_MJ_DEVICE_CONTROL
	};

	UNREFERENCED_PARAMETER(puniszRegistryPath);

	KdPrint(("Func: %s\n", __FUNCTION__));

	RtlInitUnicodeString(
		&uniszDevice,
		pszDevice
	);
	nt = IoCreateDevice(
		pDriverObject,
		sizeof(DEVICE_EXTENSION), &uniszDevice,
		44000, 0, FALSE,
		NULL
	);
	KdPrint(("Func %s/%s returns 0x%lX\n", __FUNCTION__, "IoCreateDevice", nt));
	if (!NT_SUCCESS(nt)) {
		goto fail;
	}

	RtlInitUnicodeString(
		&uniszSymlink,
		pszSymlink
	);
	nt = IoCreateSymbolicLink(
		&uniszSymlink,
		&uniszDevice
	);
	KdPrint(("Func %s/%s returns 0x%lX\n", __FUNCTION__, "IoCreateSymbolicLink", nt));
	if (!NT_SUCCESS(nt)) {
		goto fail;
	}

	pDriverObject->DriverUnload = OnDrvUnload;
	for (ULONG i = 0; i < ARRAYSIZE(iIrpCodes); ++i)
		pDriverObject->MajorFunction[iIrpCodes[i]] = DispatchDriver;

fail:
	KdPrint(("Func %s returns 0x%lX\n", __FUNCTION__, nt));
	return nt;
}