#include "kss2-adaptiv.h"

#include <cstdarg>
#include <string.h>
#include <array>

#include "../common.h"

/*
* Trenutno je ostalo da se zavrsi:
    load_interstage_sequences - u xml-u postoji 56 tagova a vrednost odgovarajuceg makra je 16,
    load_adaptiv_opeartion_variables - objasnjenje je u f-ji
*/

namespace kss2_adaptiv
{

static constexpr uint ATTR_VAL = 0b000000001;
static constexpr uint TXT_VAL = 0b000000010;

inline static int get_number_from_dxml(dxml_t node, const uint mask, ...)
{
    std::va_list args;
    va_start(args, mask);

    if (mask & ATTR_VAL)
    {
        const char *attr_name = va_arg(args, const char *);
        return std::atoi(dxml_attr(node, attr_name));
    }
    else if (mask & TXT_VAL)
    {
        return std::atoi(node->txt);
    }

    va_end(args);
    return 0;
}

inline static void fill_lower_higher_int(dxml_t xml, uchar &lower, uchar &higher)
{
    int val = get_number_from_dxml(xml, TXT_VAL);
    lower = (uchar)(val);
    higher = (uchar)(val >> 8);
}

static void load_stage(dxml_t xml, SFaza &stage)
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
        TXT_VAL);

    auto stage = dxml_child(xml, "STAGE");
    for (int i = 0; stage && i < BR_FAZA; stage = stage->next)
        load_stage(stage, faze[i++]);
}

static void load_interstage_sequences(dxml_t xml, SPrelaz sequences[BR_PRELAZA])
{
    auto interstage_sequences = dxml_child(xml, "INTERSTAGE_SEQUENCES");

    int no_sequences = get_number_from_dxml(
        dxml_child(interstage_sequences, "NUMBER_OF_INTERSTAGE_SEQUENCES"),
        TXT_VAL);

    for (int i = 0; i < BR_PRELAZA; ++i)
    {
        auto sequence = dxml_child(interstage_sequences, "INTERSTAGE_SEQUENCE");
        auto idx = atoi(dxml_attr(sequence, "NO")) - 1;
        auto &sekvenca = sequences[idx];

        sekvenca.broj_sadasnje = get_number_from_dxml(dxml_child(sequence, "CURRENT_STAGE"), TXT_VAL);
        sekvenca.broj_naredne = get_number_from_dxml(dxml_child(sequence, "NEXT_STAGE"), TXT_VAL);
        sekvenca.duzina = get_number_from_dxml(dxml_child(sequence, "DURATION"), TXT_VAL);

        auto toggle_secs = dxml_child(sequence, "TOGGLE_SECONDS");
        for (int j = 0; j < TOGGLE_COUNT; ++j)
        {
            auto toggle = dxml_child(toggle_secs, "toggle_sec");
            int idx = atoi(dxml_attr(toggle, "NO")) - 1;
            sekvenca.toggle[idx] = get_number_from_dxml(toggle, TXT_VAL);
        }

        fill_lower_higher_int(dxml_child(sequence, "INTERSTAGE_SEQUENCE_CRC"), sekvenca.CRC_prelaza_l,
                              sekvenca.CRC_prelaza_h);
    }
}

template <typename T>
inline static void assign_xml_values(dxml_t xml, const char *name, T &value)
{
    value = get_number_from_dxml(
        dxml_child(xml, name),
        TXT_VAL);
}

static void load_stage(dxml_t xml, SParametriFaze &stage)
{
    assign_xml_values(xml, "stage_number", stage.broj);
    fill_lower_higher_int(dxml_child(xml, "stage_duration"), stage.opt_l, stage.opt_h);
}

static void load_stage_change_plan(dxml_t xml, SPlanIzmenaFaza &plan)
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
        TXT_VAL);

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
    for (; description; description = description->sibling)
    {
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
        TXT_VAL);

    dxml_t detektor = dxml_child(detector_config, "DETECTOR");
    for (; detektor; detektor = detektor->next)
    {
        auto detektor_idx = get_number_from_dxml(detektor, ATTR_VAL, "NO");
        if (detektor_idx > BR_DETEKTORA)
            printf("Los NO atribut detektora\n\r");
        load_detektor(detektor, detektori[detektor_idx].detector_description);
    }
}

static void load_adaptiv_operation_variables(dxml_t xml, uint data[16][32])
{
    for (auto variable = dxml_child(xml, "VARIABLES"); variable; variable = variable->next)
    {
        int idx = get_number_from_dxml(variable, ATTR_VAL, "NO") - 1;
        auto var = data[idx];

        auto system = dxml_child(variable, "SYSTEM");
        int i = 0;
        for (auto ordered = dxml_child(system, "START_STAGE"); ordered && i < 4; ordered = ordered->ordered, ++i)
            var[i] = get_number_from_dxml(ordered, TXT_VAL);
        
        dxml_t tfree = dxml_child(variable, "FREE");
        for (auto v = dxml_child(tfree, "VARIABLE"); v; v = v->next)
            var[get_number_from_dxml(v, ATTR_VAL, "NO") - 1] = get_number_from_dxml(v, TXT_VAL);
    }
}

static void load_nodes(dxml_t xml, algo_stage &stage)
{
    auto no_nodes = get_number_from_dxml(dxml_child(xml->parent, "NUMBER_OF_NODES"),
                                         TXT_VAL);

    dxml_t node = dxml_child(xml->parent, "NODE");
    for (; node; node = node->next)
    {
        auto n_idx = get_number_from_dxml(node, ATTR_VAL, "NO");
        SA_cvor &current = stage[n_idx];
        current.uslov = get_number_from_dxml(dxml_child(node, "CONDITION_TYPE"), TXT_VAL);
        current.uslov_data = get_number_from_dxml(dxml_child(node, "CONDITION_JOKER"), TXT_VAL);

        dxml_t data = dxml_child(node, "CONDITION_DATA");
        for (auto d = dxml_child(data, "data"); d; d = d->next)
        {
            auto d_idx = get_number_from_dxml(d, ATTR_VAL, "NO");
            current.U.data[d_idx] = get_number_from_dxml(d, TXT_VAL);
        }

        current.fun = get_number_from_dxml(dxml_child(node, "ACTION_TYPE"), TXT_VAL);
        current.fun_data = get_number_from_dxml(dxml_child(node, "ACTION_JOKER"), TXT_VAL);
        data = dxml_child(node, "ACTION_DATA");
        for (auto d = dxml_child(data, "data"); d; d = d->next)
        {
            auto d_idx = get_number_from_dxml(d, ATTR_VAL, "NO");
            current.F.data[d_idx] = get_number_from_dxml(d, TXT_VAL);
        }

        dxml_t next = dxml_child(node, "NEXT_NODE");
        current.u_SA.sledeci.pozitivan = get_number_from_dxml(dxml_child(next, "true_next"), TXT_VAL);
        current.u_SA.sledeci.negativan = get_number_from_dxml(dxml_child(next, "false_next"), TXT_VAL);
        current.u_SA.sledeci.p_end = get_number_from_dxml(dxml_child(next, "true_end"), TXT_VAL);
        current.u_SA.sledeci.n_end = get_number_from_dxml(dxml_child(next, "false_end"), TXT_VAL);
        current.u_SA.sledeci.rezerva = get_number_from_dxml(dxml_child(next, "reserve"), TXT_VAL);
    }
}

static void load_operation_algorithms(dxml_t xml, algo_stage stages[BR_ALG_FAZA])
{
    dxml_t adaptiv = dxml_child(xml, "ADAPTIV_OPERATION_ALGORITHMS_FOR_STAGES");
    auto no_stages = get_number_from_dxml(dxml_child(adaptiv, "NUMBER_OF_STAGES"),
                                          TXT_VAL);

    dxml_t stage = dxml_child(adaptiv, "STAGE");
    for (; stage; stage = stage->next)
    {
        auto s_idx = get_number_from_dxml(stage, ATTR_VAL, "NO");
        load_nodes(dxml_child(stage, "NODE"), stages[s_idx]);
    }
}

static void load_op_algos_for_interstages(dxml_t xml, InterstageAlgo interstages[BR_ALG_PRELAZA])
{
    dxml_t inters = dxml_child(xml, "ADAPTIV_OPERATION_ALGORITHMS_FOR_INTERSTAGES");
    auto no_inters = get_number_from_dxml(dxml_child(inters, "NUMBER_OF_INTERSTAGES"),
                                          TXT_VAL);

    dxml_t istage = dxml_child(inters, "INTERSTAGE");
    for (; istage; istage = istage->next)
    {
        auto i_idx = get_number_from_dxml(istage, ATTR_VAL, "NO");
        load_nodes(dxml_child(istage, "NODE"), interstages[i_idx].nodes);
        interstages[i_idx].crc = get_number_from_dxml(dxml_child(istage, "CRC_OF_INTERSTAGE_ALGORITHM"),
                                                      TXT_VAL);
    }

    auto crc = get_number_from_dxml(dxml_child(inters, "CRC_OF_ALGORITHMS_FOR_INTERSTAGES"),
                                    TXT_VAL);
}

void load(const char *filename, SFaza faze[BR_FAZA], SPrelaz sequences[BR_PRELAZA],
         SPlanIzmenaFaza change_plans[BR_FAZA], DET_descr detektori[BR_DETEKTORA], 
         uint data[16][32], 
        kss2_adaptiv::algo_stage stages[BR_FAZA], InterstageAlgo interstages[BR_ALG_PRELAZA])
{
    auto fp = fopen(filename, "r");
    auto xml = dxml_parse_fp(fp);

    load_fases(xml, faze);
    load_interstage_sequences(xml, sequences);
    load_stage_change_plans(xml, change_plans);
    load_detector_config(xml, detektori);
    load_adaptiv_operation_variables(xml, data);
    load_operation_algorithms(xml, stages);
    load_op_algos_for_interstages(xml, interstages);
}

#define CREATE_TAG(xml, var_name, name) auto var_name = dxml_add_child(xml, name, 0)

inline static int int_from_higher_lower(const uchar higher, const uchar lower)
{
    int rv = higher;
    rv <<= 8;
    rv += lower;
    return rv;
}

static void save_stage(dxml_t xml, const SFaza &faza, int current)
{
    dxml_t stage = create_tag_with_attr(xml, "STAGE", "NO", current + 1);

    create_tag_with_txt(stage, "STAGE_NUMBER", faza.broj);

    CREATE_TAG(stage, duration, "STAGE_DURATION");

    create_tag_with_txt(duration, "min", int_from_higher_lower(faza.min_h, faza.min_l));
    create_tag_with_txt(duration, "opt", int_from_higher_lower(faza.opt_h, faza.opt_l));
    create_tag_with_txt(duration, "max", int_from_higher_lower(faza.max_h, faza.max_l));

    // </STAGE_DURATION>

    CREATE_TAG(stage, colors, "STAGE_COLORS");

    /*
        Za testiranje for-a koji je ispod

        ********************************
        #include <bits/stdc++.h>

        using uchar = unsigned char;

        int main()
        {
            uchar boje[4] = { 0b00000001, 0b00000010, 0b00000100, 0b00001000 };

            for (int i = 0, idx = 0; i < 32; ++i)
            {
                if (i == 8 || i == 16 || i == 24)
                    ++idx;
                auto bit = i - 8 * idx;
                auto col = (boje[idx] &  (1 << (bit - 1))) >> (bit - 1);
                std::printf("%d: %d -> %d\n", idx, bit, col);
            }
            return 0;
        }
        ********************************
    */

    for (int i = 0, idx = 0; i < BR_BOJA; ++i)
    {
        // Avoiding % operator
        if (i == 8 || i == 16 || i == 24)
            ++idx;

        auto bit = i - 8 * idx;
        auto col = (faza.boje[idx] & (1 << (bit - 1))) >> (bit - 1);
        create_tag_with_attr_and_txt(colors, "collor_of_signal_group", "NO", i + 1, col);
    }

    // </STAGE_COLORS>

    create_tag_with_txt(stage, "STAGE_CRC", int_from_higher_lower(faza.CRC_faze_h, faza.CRC_faze_l));
}

static void save_interstage(dxml_t xml, const SPrelaz &prelaz, int current)
{
    dxml_t sequence = create_tag_with_attr(xml, "INTERSTAGE_SEQUENCE", "NO", current + 1);

    create_tag_with_txt(sequence, "CURRENT_STAGE", prelaz.broj_sadasnje);
    create_tag_with_txt(sequence, "NEXT_STAGE", prelaz.broj_naredne);
    create_tag_with_txt(sequence, "DURATION", prelaz.duzina);

    dxml_t toggle_secs = dxml_add_child(sequence, "TOGGLE_SECONDS", 0);
    for (int i = 0; i < TOGGLE_COUNT; ++i)
        create_tag_with_attr_and_txt(toggle_secs, "toggle_sec", "NO", i + 1, prelaz.toggle[i]);

    create_tag_with_txt(sequence, "INTERSTAGE_SEQUENCE_CRC", int_from_higher_lower(prelaz.CRC_prelaza_h, prelaz.CRC_prelaza_l));
}

static void save_stage_change_plan(dxml_t xml, const SPlanIzmenaFaza &plan, int current)
{
    auto change_plan = create_tag_with_attr(xml, "STAGE_CHANGE_PLAN", "NO", current + 1);

    create_tag_with_txt(change_plan, "STAGE_CHANGE_PLAN_NUMBER", plan.broj_PIFa);
    create_tag_with_txt(change_plan, "NUMBER_OF_STAGES", plan.broj_faza);
    create_tag_with_txt(change_plan, "CYCLE", plan.duzina_cyc);
    create_tag_with_txt(change_plan, "START_SECOND", plan.start_sec);

    auto stages = dxml_add_child(change_plan, "STAGES", 0);
    for (int i = 0; i < BR_FAZA; ++i)
    {
        auto stage = create_tag_with_attr(stages, "STAGE", "NO", i + 1);
        create_tag_with_txt(stage, "stage_number", plan.faze[i].broj);
        create_tag_with_txt(stage, "stage_duration", int_from_higher_lower(plan.faze[i].opt_h, plan.faze[i].opt_l));
    }
    create_tag_with_txt(change_plan, "STAGE_CHANGE_PLAN_CRC", int_from_higher_lower(plan.CRC_PlanIzmena_h, plan.CRC_PlanIzmena_l));
}

static void save_detector(dxml_t xml, const DET_descr &detektor, int current)
{
    auto detector = create_tag_with_attr(xml, "DETECTOR", "NO", current + 1);

    const char *base_tag_str = "detector_description_";
    auto base_len = strlen(base_tag_str);

    for (int i = 0; i < 16; ++i)
    {
        auto len = base_len + 1;
        if (i < 9)
            ++len;
        else
            len += 2;

        auto tag_name = (char *)malloc(len * sizeof(char));
        memset(tag_name, 0, len);
        snprintf(tag_name, len, base_tag_str, i + 1);

        create_tag_with_txt(detector, tag_name, detektor.detector_description[i]);
    }
}

static void save_op_variable(dxml_t xml, const uint *variable, int current)
{
    static const std::array<const char *, 4> system_vars = {"START_STAGE",
                                                            "COORDINATION_STAGE",
                                                            "CYCLE",
                                                            "START_SECOND"};

    auto variables = create_tag_with_attr(xml, "VARIABLES", "NO", current + 1);
    auto system = dxml_add_child(variables, "SYSTEM", 0);
    int i = 0;
    for (const auto& var_name : system_vars)
        create_tag_with_txt(system, var_name, variable[i++]);
    
    auto tfree = dxml_add_child(variables, "FREE", 0);

    for (int i = 4; i < 32; ++i)
        create_tag_with_attr_and_txt(tfree, "VARIABLE", "NO", i + 1, variable[i]);
}

static void save_algo_stage(dxml_t xml, const algo_stage &faza, int current)
{
    auto stage = create_tag_with_attr(xml, "STAGE", "NO", current + 1);

    create_tag_with_txt(stage, "NUMBER_OF_NODES", BR_CVOROVA_AF);

    for (int i = 0; i < BR_CVOROVA_AF; ++i)
    {
        auto node = create_tag_with_attr(stage, "NODE", "NO", i + 1);
        create_tag_with_txt(node, "CONDITION_TYPE", faza[i].uslov);
        create_tag_with_txt(node, "CONDITION_JOKER", faza[i].uslov_data);

        dxml_t condition_data = dxml_add_child(node, "CONTITION_DATA", 0);
        for (int j = 0; j < 6; ++j)
            create_tag_with_attr_and_txt(condition_data, "data", "NO", j + 1, faza[i].U.data[j]);
        // </CONDITION_DATA>

        create_tag_with_txt(node, "ACTION_TYPE", faza[i].fun);
        create_tag_with_txt(node, "ACTION_JOKER", faza[i].fun_data);
        dxml_t action_data = dxml_add_child(node, "ACTION_DATA", 0);
        for (int j = 0; j < 6; ++j)
            create_tag_with_attr_and_txt(action_data, "data", "NO", j + 1, faza[i].F.data[j]);
        // </CONDITION_DATA>

        dxml_t next_node = dxml_add_child(node, "NEXT_NODE", 0);
        create_tag_with_txt(next_node, "true_next", faza[i].u_SA.sledeci.pozitivan);
        create_tag_with_txt(next_node, "false_next", faza[i].u_SA.sledeci.negativan);
        create_tag_with_txt(next_node, "true_end", faza[i].u_SA.sledeci.p_end);
        create_tag_with_txt(next_node, "false_end", faza[i].u_SA.sledeci.n_end);
        create_tag_with_txt(next_node, "reserve", faza[i].u_SA.sledeci.rezerva);
        // </NEXT_NODE>
    }

    create_tag_with_txt(stage, "CRC_OF_STAGE_ALGORITHM", 0); // Nemam strukturu kojom mogu da popunim ovo polje
}

static void save_algo_interstage(dxml_t xml, const InterstageAlgo &prelaz, int current)
{
    auto interstage = create_tag_with_attr(xml, "INTERSTAGE", "NO", current + 1);

    create_tag_with_txt(interstage, "NUMBER_OF_NODES", BR_CVOROVA_AF);

    for (int i = 0; i < BR_CVOROVA_AF; ++i)
    {
        auto node = create_tag_with_attr(interstage, "NODE", "NO", i + 1);
        create_tag_with_txt(node, "CONDITION_TYPE", prelaz.nodes[i].uslov);
        create_tag_with_txt(node, "CONDITION_JOKER", prelaz.nodes[i].uslov_data);

        dxml_t condition_data = dxml_add_child(node, "CONTITION_DATA", 0);
        for (int j = 0; j < 6; ++j)
            create_tag_with_attr_and_txt(condition_data, "data", "NO", j + 1, prelaz.nodes[i].U.data[j]);
        // </CONDITION_DATA>

        create_tag_with_txt(node, "ACTION_TYPE", prelaz.nodes[i].fun);
        create_tag_with_txt(node, "ACTION_JOKER", prelaz.nodes[i].fun_data);
        dxml_t action_data = dxml_add_child(node, "ACTION_DATA", 0);
        for (int j = 0; j < 6; ++j)
            create_tag_with_attr_and_txt(action_data, "data", "NO", j + 1, prelaz.nodes[i].F.data[j]);
        // </CONDITION_DATA>

        dxml_t next_node = dxml_add_child(node, "NEXT_NODE", 0);
        create_tag_with_txt(next_node, "true_next", prelaz.nodes[i].u_SA.sledeci.pozitivan);
        create_tag_with_txt(next_node, "false_next", prelaz.nodes[i].u_SA.sledeci.negativan);
        create_tag_with_txt(next_node, "true_end", prelaz.nodes[i].u_SA.sledeci.p_end);
        create_tag_with_txt(next_node, "false_end", prelaz.nodes[i].u_SA.sledeci.n_end);
        create_tag_with_txt(next_node, "reserve", prelaz.nodes[i].u_SA.sledeci.rezerva);
        // </NEXT_NODE>
    }

    create_tag_with_txt(interstage, "CRC_OF_STAGE_ALGORITHM", prelaz.crc); // Nemam strukturu kojom mogu da popunim ovo polje
}

void save(const char *filename,
          const SFaza faze[BR_FAZA],
          const SPrelaz sequences[BR_PRELAZA],
          const SPlanIzmenaFaza change_plans[BR_FAZA],
          const DET_descr detektori[BR_DETEKTORA],
          const uint data[16][32],
          const algo_stage stages[BR_FAZA],
          const InterstageAlgo interstages[BR_ALG_PRELAZA])
{
    dxml_t xml = dxml_new("ADAPTIV_DATA");

    CREATE_TAG(xml, signal_stages, "SIGNAL_STAGES");

    for (int i = 0; i < BR_FAZA; ++i)
        save_stage(signal_stages, faze[i], i);

    // </SIGNAL_STAGES>

    CREATE_TAG(xml, interstage_sequences, "INTERSTAGE_SEQUENCES");

    create_tag_with_txt(interstage_sequences, "NUMBER_OF_INTERSTAGE_SEQUENCES", BR_PRELAZA);
    for (int i = 0; i < BR_PRELAZA; ++i)
        save_interstage(interstage_sequences, sequences[i], i);

    // </INTERSTAGE_SEQUENCES>

    CREATE_TAG(xml, stage_change_plans, "STAGE_CHANGE_PLANS");

    create_tag_with_txt(stage_change_plans, "NUMBER_OF_STAGE_CHANGE_PLANS", BR_PIFOVA);
    for (int i = 0; i < BR_PIFOVA; ++i)
        save_stage_change_plan(stage_change_plans, change_plans[i], i);

    // </STAGE_CHANGE_PLANS>

    CREATE_TAG(xml, detectors_configuration, "DETECTORS_CONFIGURATION");

    create_tag_with_txt(detectors_configuration, "NUMBER_OF_DETECTORS", BR_DETEKTORA);
    for (int i = 0; i < BR_DETEKTORA; ++i)
        save_detector(detectors_configuration, detektori[i], i);

    // </DETECTORS_CONFIGURATION>

    CREATE_TAG(xml, adaptiv_operation_variables, "ADAPTIV_OPERATION_VARIABLES");

    for (int i = 0; i < 16; ++i)
        save_op_variable(adaptiv_operation_variables, data[i], i);

    // </ADAPTIV_OPERATION_VARIABLES>

    CREATE_TAG(xml, adapiv_operation_algos_for_stages, "ADAPTIV_OPERATION_ALGORITHMS_FOR_STAGES");

    create_tag_with_txt(adapiv_operation_algos_for_stages, "NUMBER_OF_STAGES", BR_FAZA);

    for (int i = 0; i < BR_FAZA; ++i)
        save_algo_stage(adapiv_operation_algos_for_stages, stages[i], i);

    // Fali CRC_OF_ALGORITHMS_FOR_STAGES

    // </ADAPTIV_OPERATION_ALGORITHMS_FOR_STAGES>

    CREATE_TAG(xml, adaptiv_operation_algos_for_interstages, "ADAPTIV_OPERATION_ALGORITHMS_FOR_INTERSTAGES");

    create_tag_with_txt(adaptiv_operation_algos_for_interstages, "NUMBER_OF_INTERSTAGES", BR_CVOROVA_AF);

    for (int i = 0; i < BR_PRELAZA; ++i)
        save_algo_interstage(adaptiv_operation_algos_for_interstages, interstages[i], i);

    // Fali CRC_OF_ALGORITHMS_FOR_INTERSTAGES

    // </ADAPTIV_OPERATION_ALGORITHMS_FOR_INTERSTAGES>
    // </ADAPTIV_DATA>

    auto fp = fopen(filename, "w");
    auto buff = dxml_toxml(xml);
    fwrite(buff, sizeof(char), strlen(buff), fp);
    free(buff);
    fclose(fp);
}

} // namespace kss2_adaptiv