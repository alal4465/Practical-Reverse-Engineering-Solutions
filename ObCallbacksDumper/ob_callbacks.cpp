#include "ob_callbacks.hpp"


namespace ob_callbacks {
	static void dump_object_type_callbacks(POBJECT_TYPE object_type);

	static OB_PREOP_CALLBACK_STATUS dummy_callback(
		PVOID RegistrationContext,
		POB_PRE_OPERATION_INFORMATION OperationInformation
	);

	static PVOID registration_handle;
}


void ob_callbacks::dump_callbacks() {
	POBJECT_TYPE object_types[] = { 
		reinterpret_cast<ob_callbacks::POBJECT_TYPE>(*PsProcessType), 
		reinterpret_cast<ob_callbacks::POBJECT_TYPE>(*PsThreadType)
	};

	for (auto object_type : object_types) {
		dump_object_type_callbacks(object_type);
	}
}

void ob_callbacks::dump_object_type_callbacks(ob_callbacks::POBJECT_TYPE object_type) {
	KdPrint(("Dumping callbacks for object type: %wZ\n", &object_type->Name));

	ExAcquirePushLockExclusive(&object_type->TypeLock);
	for (PLIST_ENTRY callback_entry = object_type->CallbackList.Flink; callback_entry != &object_type->CallbackList; callback_entry = callback_entry->Flink) {
		auto callback = reinterpret_cast<PINTERNAL_REGISTRATION_RECORD>(callback_entry);
		KdPrint(("PreOperation: %p, PostOperation: %p\n", callback->PreOperation, callback->PostOperation));
	}
	ExReleasePushLockExclusive(&object_type->TypeLock);
}

bool ob_callbacks::register_dummy_callback() {
	OB_OPERATION_REGISTRATION operation_registration{ 0 };
	operation_registration.ObjectType = PsProcessType;
	operation_registration.PostOperation = nullptr;
	operation_registration.PreOperation = ob_callbacks::dummy_callback;
	operation_registration.Operations = OB_OPERATION_HANDLE_CREATE;

	OB_CALLBACK_REGISTRATION callback_registration{ 0 };
	callback_registration.Version = OB_FLT_REGISTRATION_VERSION;
	callback_registration.OperationRegistrationCount = 1;
	callback_registration.Altitude = RTL_CONSTANT_STRING(L"alt");
	callback_registration.RegistrationContext = 0;
	callback_registration.OperationRegistration = &operation_registration;

	return NT_SUCCESS(ObRegisterCallbacks(&callback_registration, &registration_handle));
}

void ob_callbacks::unregister_dummy_callback() {
	ObUnRegisterCallbacks(ob_callbacks::registration_handle);
}

OB_PREOP_CALLBACK_STATUS ob_callbacks::dummy_callback(PVOID RegistrationContext, 
	POB_PRE_OPERATION_INFORMATION OperationInformation) {
	UNREFERENCED_PARAMETER(RegistrationContext);
	UNREFERENCED_PARAMETER(OperationInformation);
	return OB_PREOP_SUCCESS;
}
