#include "pch.h"

#include "mod_sa_fix.h"

urmem::hook gta_vehicle_get_by_id_hk{};
mod_sa_fix::vehicle_entry* __cdecl HOOK_gta_vehicle_get_by_id(int id);

void mod_sa_fix::Init() {
    if (!Plugin.GetSettings().mod_sa_fix_enabled()) {
        return;
    }

    for (const auto& mod_sa_version : Plugin.GetSettings().get_mod_sa_versions()) {
        module_t mod_sa_module = module_t(mod_sa_version.module_name.c_str());
        if (!mod_sa_module.get_address()) {
            continue;
        }

        if (memcmp_safe((uint8_t*)mod_sa_module.get_address() + mod_sa_version.detect_offset, mod_sa_version.detect_signature.data(),
            mod_sa_version.detect_signature.size())) {

            Plugin.Log("Detected mod_sa version: {}", mod_sa_version.name);
            this->mod_sa_version = mod_sa_version;
            this->mod_sa_module = mod_sa_module;
            inited = true;
            break;
        }
    }

    if (!inited) {
        Plugin.Log("Couldn't detect any mod_sa version!");
    }
    else {
        Plugin.Log("Finding function gta_vehicle_get_by_id...");

        urmem::sig_scanner sig{};
        sig.init(mod_sa_module.get_address());

        urmem::address_t addr{};
        if (!sig.find(mod_sa_version.gta_vehicle_get_by_id_pattern.pattern.c_str(),
            mod_sa_version.gta_vehicle_get_by_id_pattern.mask.c_str(), addr)) {
            Plugin.Log("Couldn't find function gta_vehicle_get_by_id!");
            return;
        }

        Plugin.Log("Hooking gta_vehicle_get_by_id (offset: {:#x})...", addr);
        gta_vehicle_get_by_id_hk.install(addr, urmem::get_func_addr(&HOOK_gta_vehicle_get_by_id), urmem::hook::type::jmp, 9);
    }
}

void mod_sa_fix::OnAddNewVehicle(void* pVehicle) {
    if (!inited || !pVehicle) {
        return;
    }

    const auto* samp_vehicle = reinterpret_cast<samp_vehicle_t*>(pVehicle);
    if (samp_vehicle->pGtaVehicle) {

        const auto model_id = samp_vehicle->pGtaVehicle->entity.modelId;
        if (is_default_model(model_id)) {
            return;
        }

        if (entries.find(model_id) != entries.end()) {
            return;
        }

        auto _class = eVehicleClass::VEHICLE_CLASS_CAR;
        auto name = VehicleNames.GetPtr(model_id);
        //Plugin.Log("name: {}", name);

        switch (samp_vehicle->pGtaVehicle->m_nVehicleType) {
        case eVehicleType::VEHICLE_TYPE_BIKE:
        case eVehicleType::VEHICLE_TYPE_BMX:
        case eVehicleType::VEHICLE_TYPE_QUAD: {
            _class = eVehicleClass::VEHICLE_CLASS_BIKE;
            break;
        }
        case eVehicleType::VEHICLE_TYPE_HELI:
        case eVehicleType::VEHICLE_TYPE_FHELI: {
            _class = eVehicleClass::VEHICLE_CLASS_HELI;
            break;
        }
        case eVehicleType::VEHICLE_TYPE_PLANE:
        case eVehicleType::VEHICLE_TYPE_FPLANE: {
            _class = eVehicleClass::VEHICLE_CLASS_AIRPLANE;
            break;
        }
        case eVehicleType::VEHICLE_TYPE_BOAT: {
            _class = eVehicleClass::VEHICLE_CLASS_BOAT;
            break;
        }
        case eVehicleType::VEHICLE_TYPE_TRAIN: {
            _class = eVehicleClass::VEHICLE_CLASS_HEAVY;
            break;
        }
        case eVehicleType::VEHICLE_TYPE_TRAILER: {
            _class = eVehicleClass::VEHICLE_CLASS_TRAILER;
            break;
        }
        }

        add_entry(model_id, _class, samp_vehicle->pGtaVehicle->m_nMaxPassengers, name);
    }
}

mod_sa_fix::vehicle_entry* mod_sa_fix::get_entry(int modelId) {
    auto p_entry = entries.find(modelId);
    if (p_entry != entries.end()) {
        return p_entry->second;
    }
    return entries.at(0);
}

__declspec(noinline) mod_sa_fix::vehicle_entry* call_get_entry(int id) {

    if (!mod_sa_fix::is_default_model(id)) {
        return mod_sa_fix.get_entry(id);
    }
    return nullptr;
}

mod_sa_fix::vehicle_entry* __cdecl HOOK_gta_vehicle_get_by_id(int id) {
    auto entry = call_get_entry(id);
    if (entry) {
        return entry;
    }

    id -= 400;
    return urmem::call_function<urmem::calling_convention::cdeclcall, mod_sa_fix::vehicle_entry*, int>(
        gta_vehicle_get_by_id_hk.get_original_addr() + 9, id);
}