#include "kss2-adaptiv.h"

#include <cstdarg>
#include <string.h>

#include "dxml.h"

namespace kss2_adaptiv
{

static constexpr uint ATTR_VAL = 0b000000001;
static constexpr uint TXT_VAL = 0b000000010;

inline static int get_number_from_dxml(dxml_t node, const uint mask, ...)
{
    std::va_list args;
    va_start(args, node);

    if (mask & ATTR_VAL) {
        const char* attr_name = va_arg(args, const char*);
        return std::atoi(dxml_attr(node, attr_name));
    } else if (mask & TXT_VAL) {
        return std::atoi(node->txt);
    }

    va_end(args);
    return 0;
}

inline static void fill_lower_higher_int(dxml_t xml, uchar& lower, uchar& higher)
{
    int val = get_number_from_dxml(xml, TXT_VAL);
    lower = (uchar)(val);
    higher = (uchar)(val >> 8);
}

static void load_stage(dxml_t xml, SFaza& stage)
{
    stage.broj = get_number_from_dxml(dxml_child(xml, "STAGE_NUMBER"), TXT_VAL);
    
    auto duration = dxml_child(xml, "STAGE_DURATION");

    fill_lower_higher_int(dxml_child(duration, "min"), stage.min_l, stage.min_h);
    fill_lower_higher_int(dxml_child(duration, "opt"), stage.opt_l, stage.opt_h);
    fill_lower_higher_int(dxml_child(duration, "max"), stage.max_l, stage.max_h);

    auto colors = dxml_child(xml, "STAGE_COLORS");
    // collor_of_signal_group
    auto cosg = dxml_child(colors, "collor_of_signal_group");
    for (int i = 0, idx = 0; cosg && i < BR_BOJA; cosg = cosg->next, ++i)
    {
        // Avoiding % operator
        if (i == 8 || i == 16 || i == 24)
            ++idx;

        int cosg_idx = get_number_from_dxml(cosg, ATTR_VAL, "NO");
        int color = get_number_from_dxml(cosg, TXT_VAL);
        stage.boje[idx] |= color << (cosg_idx - idx * 8); // Avoiding % operator
    }

    fill_lower_higher_int(dxml_child(xml, "STAGE_CRC"), stage.CRC_faze_l, stage.CRC_faze_h);
}

static void load_fases(dxml_t xml, SFaza faze[BR_FAZA])
{
    auto signal_stages = dxml_child(xml, "SIGNAL_STAGES");

    int no_signal_stages = get_number_from_dxml(
        dxml_child(signal_stages, "NUMBER_OF_SIGNAL_STAGES"), 
        TXT_VAL
    );

    auto stage = dxml_child(xml, "STAGE");
    for (int i = 0; stage && i < BR_FAZA; stage = stage->next)
        load_stage(stage, faze[i++]);
}

static void load_interstage_sequences(dxml_t xml, SPrelaz sequences[BR_PRELAZA])
{
    auto interstage_sequences = dxml_child(xml, "INTERSTAGE_SEQUENCES");

    int no_sequences = get_number_from_dxml(
        dxml_child(interstage_sequences, "NUMBER_OF_INTERSTAGE_SEQUENCES"),
        TXT_VAL
    );

    /// TO-DO
    /// Proveri sa Rasom da li je broj od 1 do 16 ili od 1 do 56

}

template<typename T>
inline static void assign_xml_values(dxml_t xml, const char* name, T& value)
{
    value = get_number_from_dxml(
        dxml_child(xml, name),
        TXT_VAL
    );
}

static void load_stage(dxml_t xml, SParametriFaze& stage)
{
    assign_xml_values(xml, "stage_number", stage.broj);
    fill_lower_higher_int(dxml_child(xml, "stage_duration"), stage.opt_l, stage.opt_h);
}

static void load_stage_change_plan(dxml_t xml, SPlanIzmenaFaza& plan)
{
    assign_xml_values(xml, "STAGE_CHANGE_PLAN_NUMBER", plan.broj_PIFa);
    assign_xml_values(xml, "NUMBER_OF_STAGES", plan.broj_faza);
    assign_xml_values(xml, "CYCLE", plan.duzina_cyc);
    assign_xml_values(xml, "START_SECOND", plan.start_sec);
    
    dxml_t stage = dxml_child(dxml_child(xml, "STAGES"), "STAGE");
    for (int i = 0; stage && i < BR_FAZA_uPIFu; stage = stage->next, ++i)
        load_stage(stage, plan.faze[i]);

    fill_lower_higher_int(dxml_child(xml, "STAGE_CHANGE_PLAN_CRC"), 
        plan.CRC_PlanIzmena_l, plan.CRC_PlanIzmena_h);
}

static void load_stage_change_plans(dxml_t xml, SPlanIzmenaFaza change_plans[BR_PIFOVA])
{
    auto stage_change_plans = dxml_child(xml, "STAGE_CHANGE_PLANS");
    int no_change_plans = get_number_from_dxml(
        dxml_child(stage_change_plans, "STAGE_CHANGE_PLAN_NUMBER"),
        TXT_VAL
    );

    auto plan = dxml_child(stage_change_plans, "STAGE_CHANGE_PLAN");
    for (int i = 0; plan && i < BR_PIFOVA; plan = plan->next, ++i)
        if (get_number_from_dxml(plan, ATTR_VAL, "NO") < BR_PIFOVA)
            load_stage_change_plan(plan, change_plans[get_number_from_dxml(plan, ATTR_VAL, "NO")]);
        else
            break;
}

static void load_detektor(dxml_t xml, uchar detektor[16])
{
    // char* buffer = new char[strlen("detector_description_16") + 1];
    auto description = dxml_child(xml, "detector_description_1");
    for (; description; description = description->sibling) {
        auto number_start = strchr(strchr(description->name, '_') + 1, '_') + 1;
        uint idx = std::atoi(number_start);
        detektor[idx] = get_number_from_dxml(description, TXT_VAL);
    }
}

static void load_detector_config(dxml_t xml, DET_descr detektori[BR_DETEKTORA])
{
    auto detector_config = dxml_child(xml, "DETECTORS_CONFIGURATION");
    auto no_detektora = get_number_from_dxml(
        dxml_child(detector_config, "NUMNER_OF_DETEKTORS"),
        TXT_VAL
    );

    dxml_t detektor = dxml_child(detector_config, "DETECTOR");
    for (; detektor; detektor = detektor->next)
    {
        auto detektor_idx = get_number_from_dxml(detektor, ATTR_VAL, "NO");
        if (detektor_idx > BR_DETEKTORA)
            printf("Los NO atribut detektora\n\r");
        load_detektor(detektor, detektori[detektor_idx].detector_description);
    }
}

}