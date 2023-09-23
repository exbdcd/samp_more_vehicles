#pragma once

inline class mod_sa_fix {
public:
    __forceinline static bool is_default_model(int model) {
        return model >= 400 && model <= 611;
    }

public:
    enum eVehicleClass : int {
        VEHICLE_CLASS_CAR = 0,
        VEHICLE_CLASS_CAR_FAST,
        VEHICLE_CLASS_HEAVY,
        VEHICLE_CLASS_HELI,
        VEHICLE_CLASS_AIRPLANE,
        VEHICLE_CLASS_BIKE,
        VEHICLE_CLASS_BOAT,
        VEHICLE_CLASS_MINI,
        VEHICLE_CLASS_TRAILER
    };

    struct vehicle_entry {
        int             id;             // model id
        int             class_id;       // class id
        char*           name;           // vehicle name
        int             passengers;     // total passenger seats, 0-10
    };

#pragma pack(push, 1)
    struct gta_entity_t {
        uint32_t vtable;
        uint8_t __pad1[20];
        uint8_t __pad2[10];
        uint16_t modelId;
        uint8_t __pad3[20];
        uint8_t __pad4[256];

        void test() {
            constexpr size_t i = sizeof(mod_sa_fix::gta_entity_t);
        }
    };
    VALIDATE_SIZE(mod_sa_fix::gta_entity_t, 0x138);

    enum eVehicleType : int {
        VEHICLE_TYPE_IGNORE = -1,
        VEHICLE_TYPE_AUTOMOBILE = 0,
        VEHICLE_TYPE_MTRUCK = 1,  // MONSTER TRUCK
        VEHICLE_TYPE_QUAD = 2,
        VEHICLE_TYPE_HELI = 3,
        VEHICLE_TYPE_PLANE = 4,
        VEHICLE_TYPE_BOAT = 5,
        VEHICLE_TYPE_TRAIN = 6,
        VEHICLE_TYPE_FHELI = 7,
        VEHICLE_TYPE_FPLANE = 8,
        VEHICLE_TYPE_BIKE = 9,
        VEHICLE_TYPE_BMX = 10,
        VEHICLE_TYPE_TRAILER = 11
    };

    struct gta_vehicle_t {
        gta_entity_t entity;
        uint8_t __pad1[0x328];
        uint8_t __pad2[40];
        uint8_t m_nMaxPassengers;
        uint8_t __pad3[0x107];
        eVehicleType m_nVehicleType;
        eVehicleType m_nVehicleSubType;
        uint8_t __pad4[0x8];
 
    };
    VALIDATE_OFFSET(mod_sa_fix::gta_vehicle_t, m_nMaxPassengers, 1160);
    VALIDATE_SIZE(mod_sa_fix::gta_vehicle_t, 0x5A0);

    struct samp_entity_t {
        uint32_t vtable;
        uint8_t __pad1[60];
        gta_entity_t* pGtaEntity;
        uint32_t gtaEntityHandle;
    };

    struct samp_vehicle_t {
        samp_entity_t samp_entity;
        uint8_t __pad1[4];
        gta_vehicle_t* pGtaVehicle;
        uint8_t __pad2[55];
    };
#pragma pack(pop)

public:
    mod_sa_fix() {
        static char default_name[] = "Unknown";
        add_entry(0, VEHICLE_CLASS_CAR, 0, default_name);
    }

    void Init();
    void OnAddNewVehicle(void* pVehicle);

    vehicle_entry* get_entry(int modelId);

private:
    void add_entry(int modelId, eVehicleClass _class, int passengers, char* name) {
        auto new_entry = new vehicle_entry;
        new_entry->id = modelId;
        new_entry->class_id = _class;
        new_entry->passengers = passengers;
        new_entry->name = name;
        entries.emplace(modelId, new_entry);
    }

private:
    bool inited{};
    module_t mod_sa_module{};
    mod_sa_version_t mod_sa_version{};
    std::unordered_map<int, vehicle_entry*> entries{};
} mod_sa_fix;