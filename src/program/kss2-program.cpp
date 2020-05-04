#include "kss2-program.h"

#include "string.h"

#include "../common.h"

#define GET_TXT_VALUE_FROM_CHILD(parent, child) atoi(dxml_child(parent, child)->txt)

bool ellement_in_array(int el, int *arr, int n)
{
    for(int i = 0; i < n; i++)
        if (el == arr[i])
            return true;
    return false;
}

namespace kss2_program
{
int load_plan(ssplan_t *splan, dxml_t signal_plan)
{
    splan->broj_plana = atoi(dxml_attr(signal_plan, "NO"));

    splan->RBC = atoi(dxml_child(signal_plan, "RBC")->txt);
    splan->TPP = atoi(dxml_child(signal_plan, "TPP")->txt);
    splan->REF = atoi(dxml_child(signal_plan, "REF")->txt);

    dxml_t sigr; // Variable which keeps tracks of all SIGR elements in document
    char startString[10];
    char stopString[10];

    int readSIGR[NOSIGGR]; // Array that keeps track of SIGR "NO" attributes
    for (int i = 0; i < NOSIGGR; i++)
        readSIGR[i] = -1;

    int nsgr = 0;
    // citanje nodova signalnih grupa
    for (sigr = dxml_child(signal_plan, "SIGR"); sigr; sigr = sigr->next)
    {
        if (ellement_in_array(atoi(dxml_attr(sigr, "NO")) - 1, readSIGR, NOSIGGR))
            return SIGR_NUMBER_ERROR; // Two SIGR with same NUMBER
        else
            readSIGR[nsgr] = atoi(dxml_attr(sigr, "NO")) - 1;

        dxml_t start1 = dxml_child(sigr, "start1");
        dxml_t stop1 = dxml_child(sigr, "stop1");
        // provera postojanja starta1 i stopa1
        if (!start1 || !stop1)
            return SIGR_START_STOP; // No start1 or stop1

        sspar_t sp1 = {atoi(start1->txt),
                       atoi(stop1->txt)};

        for (int i = 1; i < NOSIGPAR; i++)
        {
            sprintf(startString, "start%d", i + 1);
            sprintf(stopString, "stop%d", i + 1);

            dxml_t start = dxml_child(sigr, startString);
            dxml_t stop = dxml_child(sigr, stopString);
            sspar_t sp;

            if (start && stop)
                sp = (sspar_t){atoi(start->txt),
                               atoi(stop->txt)};
            else if (start)
                sp = (sspar_t){atoi(start->txt),
                               0};
            else if (stop)
                sp = (sspar_t){0,
                               atoi(stop->txt)};
            else
                sp = (sspar_t){0, 0};
            splan->Sgrupe[atoi(dxml_attr(sigr, "NO")) - 1].Spar[i] = sp;
        }

        splan->Sgrupe[atoi(dxml_attr(sigr, "NO")) - 1].Spar[0] = sp1;
        nsgr++;
    }
    for (int i = nsgr + 1; i < NOSIGGR; i++)
    {
        for (int j = 0; j < NOSIGPAR; j++)
        {
            splan->Sgrupe[i].Spar[j] = (sspar_t){0, 0}; // Filling the rest with 0s
        }
    }

    int zbirSIGR = 0; // The sum of all SIGR "NO"s
    for (int i = 0; i < NOSIGGR; i++)
        zbirSIGR += (readSIGR[i] == -1) ? 0 : readSIGR[i]; // readSigr[i] was -1 by default which means it wasn't changed, so it has no effect on the sum

    if (zbirSIGR != nsgr * (nsgr - 1) / 2) // The sum of all numbers from 1 to n = (n*(n+1))/2, so nsgr * (nsgr - 1) / 2 represents the sum of all numbers between 1 and nsgr-1 and if our tracked sum doesn't equal that we know a SIGR "NO" attribute value was skipped.
        return SIGR_NUMBER_ERROR;

    dxml_t stop_points = dxml_child(signal_plan, "STOP_POINTS");

    if (stop_points)
    {
        dxml_t nofsp = dxml_child(stop_points, "NUMBER_OF_STOP_POINTS");
        if (nofsp)
            splan->no_stop_tacaka = atoi(nofsp->txt);
        else
            splan->no_stop_tacaka = NOSTOPTACAKA;

        dxml_t stop_p;
        int nstop_p = 0;

        for (stop_p = dxml_child(stop_points, "STOP_POINT"); stop_p; stop_p = stop_p->next)
        {
            uchar start = (dxml_child(stop_p, "start")) ? atoi(dxml_child(stop_p, "start")->txt) : 0;
            uchar max = (dxml_child(stop_p, "max")) ? atoi(dxml_child(stop_p, "max")->txt) : 0;
            uchar skok = (dxml_child(stop_p, "stop")) ? atoi(dxml_child(stop_p, "stop")->txt) : 0;

            splan->stop_tacke[atoi(dxml_attr(stop_p, "NO")) - 1].start = start;
            splan->stop_tacke[atoi(dxml_attr(stop_p, "NO")) - 1].max = max;
            splan->stop_tacke[atoi(dxml_attr(stop_p, "NO")) - 1].skok = skok;
            nstop_p++;
        }

        for (int i = nstop_p + 1; i < NOSTOPTACAKA; i++) // Filling the rest of stop points with 0s if their number in xml is smaller than NOSTOPTACAKA.
        {
            splan->stop_tacke[i].start = 0;
            splan->stop_tacke[i].max = 0;
            splan->stop_tacke[i].skok = 0;
        }
    }

    if (dxml_child(signal_plan, "SIGNAL_PLAN_CRC"))
        splan->CRC_plana = atoi(dxml_child(signal_plan, "SIGNAL_PLAN_CRC")->txt);
    else
        return NO_CRC; // No CRC

    return SIGR_NO_ERROR;
}

void load_plan_with_0s(ssplan_t *plan)
{
    plan->broj_plana = 0;
    plan->CRC_plana = 0;
    plan->RBC = 0;
    plan->REF = 0;
    plan->TPP = 0;
    plan->no_stop_tacaka = 0;
    for (int i = 0; i < NOSTOPTACAKA; i++)
    {
        plan->stop_tacke[i].max = 0;
        plan->stop_tacke[i].start = 0;
        plan->stop_tacke[i].skok = 0;
    }
    for (int i = 0; i < NOSIGGR; i++)
        for (int j = 0; j < NOSIGPAR; j++)
        {
            plan->Sgrupe[i].Spar[j].start = 0;
            plan->Sgrupe[i].Spar[j].stop = 0;
        }
}

int load_plans(ssplan_t *planovi, dxml_t xml)
{
    dxml_t root = dxml_child(xml, "SIGNAL_PLANS");

    //int nsigplans = atoi(dxml_child(root, "NUMBER_OF_SIGNAL_PLANS")->txt);

    bool filled[NOPLANS];
    for (int i = 0; i < NOPLANS; i++)
        filled[i] = false;

    for (dxml_t sp = dxml_child(root, "SIGNAL_PLAN"); sp; sp = sp->next)
    {
        int indSP;
        indSP = atoi(dxml_attr(sp, "NO")) - 1;
        if (indSP >= NOPLANS)
            continue;

        filled[indSP] = true;

        int error = load_plan(&(planovi[indSP]), sp);
        if (error != SIGR_NO_ERROR)
        {
            printf("Error occured at signal plan %d\n", ++indSP);
            return error;
        }
    }
    for (int i = 0; i < NOPLANS; i++)
        if (!filled[i])
            load_plan_with_0s(&(planovi[i]));

    return SIGR_NO_ERROR;
}

static int load_days(dxml_t timeTable, stdan_t *dani)
{
    if (timeTable == NULL)
    {
        printf("Cannot retreive time table object from file!\n");
        return TimeTableErrorInvalidFile;
    }

    dxml_t timeTableDay = dxml_child(timeTable, "TIME_TABLE_DAY");

    if (timeTableDay == NULL)
    {
        printf("Cannot retreive time table day object from file!\n");
        return TimeTableErrorInvalidFile;
    }
    int noDays = 0, sumDay = 0;
    for (dxml_t day = dxml_child(timeTableDay, "DAY"); day; day = day->next) // Loop through all of the days
    {
        int dayIdx = atoi(dxml_attr(day, "NO")) - 1;
        sumDay += dayIdx + 1;

        dani[dayIdx].dan = dayIdx + 1;

        dani[dayIdx].broj_planova = atoi(dxml_child(day, "NUMBER_OF_PLANS_PER_DAY")->txt);

        int noPlans = 0;
        for (dxml_t plan = dxml_child(day, "PLAN"); plan; plan = plan->next)
        {
            int planIdx = atoi(dxml_attr(plan, "NO")) - 1;
            if (planIdx < 0 || planIdx > 18)
            {
                printf("Plan %d in day %d has invalid number!\n", planIdx + 1, dayIdx + 1);
                return TimeTableErrorInvalidPlanNumber;
            }

            uchar sat = atoi(dxml_child(plan, "hour")->txt);
            if ((int)sat > 23 || (int)sat < 0)
            {
                printf("Plan %d in day %d has invalid hour number %d!\n", planIdx + 1, dayIdx + 1, sat);
                return TimeTableErrorInvalidValue;
            }
            dani[dayIdx].trojka[planIdx].sat = sat;

            uchar min = atoi(dxml_child(plan, "minut")->txt);
            if ((int)min > 59 || (int)min < 0)
            {
                printf("Plan %d in day %d has invalid minut number %d!\n", planIdx + 1, dayIdx + 1, min);
                return TimeTableErrorInvalidValue;
            }
            dani[dayIdx].trojka[planIdx].minut = min;

            uchar planNumber = atoi(dxml_child(plan, "plan_number")->txt);
            if ((int)planNumber > 16 || (int)planNumber < 0)
            {
                printf("Plan %d in day %d has invalid plan_number number %d!\n", planIdx + 1, dayIdx + 1, planNumber);
                return TimeTableErrorInvalidValue;
            }
            dani[dayIdx].trojka[planIdx].broj_plana = planNumber;
            noPlans++;
        }
        if (noPlans > dani[dayIdx].broj_planova)
        {
            printf("Number of plans is too large for day %d\n", dayIdx + 1);
            return TimeTableErrorInvalidValue;
        }
        else if (noPlans < dani[dayIdx].broj_planova)
        {
            printf("Number of plans is too small for day %d\n", dayIdx + 1);
            return TimeTableErrorInvalidValue;
        }
        noDays++;
    }

    if (noDays != 7)
    {
        printf("Wrong number of days.\n");
        return TimeTableErrorInvalidDays;
    }

    if (! (sumDay == 7 * 8 / 2.f)) // Checks 
    {
        printf("Some day numbers are invalid or repeated.\n");
        return TimeTableErrorInvalidDays;
    }

    return TimeTableNoErrors;
}

static int load_hollydays(dxml_t timeTable, stpraznik_t *praznik)
{
    if (praznik == NULL)
    {
        printf("No hollydays loaded(no error).\n");
        return TimeTableNoErrors;
    }

    dxml_t timeTableHol = dxml_child(timeTable, "TIME_TABLE_HOLLYDAY");

    int noHol = 0;

    praznik->broj_praznika = atoi(dxml_child(timeTableHol, "NUMBER_OF_HOLDAYS") ->txt);

    if (praznik->broj_praznika > 19 || praznik->broj_praznika < 0)
    {
        printf("Invalid number of hollydays.\n");
        return TimeTableErrorInvalidValue;
    }

    for (dxml_t holDate = dxml_child(timeTableHol, "DATE_OF_HOLIDAY"); holDate; holDate = holDate->next)
    {
        int holDateIdx = atoi(dxml_attr(holDate, "NO")) - 1;

        if (holDateIdx < 0 || holDateIdx > 18)
        {
            printf ("Error at hollyday date number %d, invalid index!\n", holDateIdx + 1);
            return TimeTableErrorInvalidValue;
        }

        int datum = atoi(dxml_child(holDate, "datum")->txt);
        int mesec = atoi(dxml_child(holDate, "mesec")->txt);

        if (mesec <= 0 || mesec > 12)
        {
            printf("Error at holyday day %d, value of mesec is %d\n", holDateIdx + 1, mesec);
            return TimeTableErrorInvalidValue;
        }

        if ((mesec <= 7 && mesec % 2 == 1) || (mesec >= 8 && mesec % 2 == 0))
        {
            if (datum <= 0 || datum > 31)
            {
                printf("Error at holyday day %d, value of mesec is %d and value of datum is %d\n", holDateIdx + 1, mesec, datum);
                return TimeTableErrorInvalidValue;
            }
        }
        else if ((mesec <= 7 && mesec % 2 == 0) || (mesec >= 8 && mesec % 2 == 1))
        {
            if (mesec != 2)
            {
                if (datum <= 0 || datum > 30)
                {
                    printf("Error at holyday day %d, value of mesec is %d and value of datum is %d\n", holDateIdx + 1, mesec, datum);
                    return TimeTableErrorInvalidValue;   
                }
            }
            else
            {
                if (datum <= 0 || datum > 28)
                {
                    printf("Error at holyday day %d, value of mesec is %d and value of datum is %d\n", holDateIdx + 1, mesec, datum);
                    return TimeTableErrorInvalidValue;
                }
            }
            
        }
        

        praznik->datum[holDateIdx].datum = datum;
        praznik->datum[holDateIdx].mesec = mesec;
        noHol++;
    }
    if (noHol > praznik->broj_praznika || noHol < praznik->broj_praznika)
    {
        printf("Invalid number of hollydays.\n");
        return TimeTableErrorInvalidValue;
    }

    int noPlans = atoi(dxml_child(timeTableHol, "NUMBER_OF_PLANS_PER_HOLIDAY")->txt);
    if (noPlans > 16 || noPlans < 0)
    {
        printf("Invalid number of plans %d", noPlans);
        return TimeTableErrorInvalidValue;
    }
    int cPlans = 0;
    for (dxml_t plan = dxml_child(timeTableHol, "PLAN"); plan; plan = plan->next)
    {
        int planIdx = atoi(dxml_attr(plan, "NO")) - 1;
        if (planIdx < 0 || planIdx > 18)
        {
            printf("Invalid plan index %d\n", planIdx);
            return TimeTableErrorInvalidValue;
        }

        praznik->planovi[planIdx].broj_plana = atoi(dxml_child(plan, "plan_number")->txt);
        praznik->planovi[planIdx].minut = atoi(dxml_child(plan, "minut")->txt);
        praznik->planovi[planIdx].sat = atoi(dxml_child(plan, "hour")->txt);
        cPlans++;
    }

    if (cPlans != noPlans)
    {
        printf("Invalid number of plans!\n");
        return TimeTableErrorInvalidValue;
    }

    return TimeTableNoErrors;
}

static int load_dates(dxml_t timeTable, stdatum_t *datumi)
{
    if (datumi == NULL)
    {
        printf("No dates loaded(no error).\n");
        return TimeTableNoErrors;
    }

    dxml_t ttd = dxml_child(timeTable, "TIME_TABLE_DATE");

    for (dxml_t date = dxml_child(ttd, "DATE_NUMBER"); date; date = date->next)
    {
        int dateIdx = atoi(dxml_attr(date, "NO")) - 9;

        if (dateIdx < 0 || dateIdx > 7)
        {
            printf("Error at special date %d, invalid value", dateIdx + 9);
            return TimeTableErrorInvalidValue;
        }

        datumi[dateIdx].broj_planova = GET_TXT_VALUE_FROM_CHILD(date, "NUMBER_OF_PLANS_PER_DATE");
        datumi[dateIdx].godina = GET_TXT_VALUE_FROM_CHILD(date, "YEAR");
        datumi[dateIdx].mesec = GET_TXT_VALUE_FROM_CHILD(date, "MONTH");
        datumi[dateIdx].datum = GET_TXT_VALUE_FROM_CHILD(date, "DATE");
        
        int nPlans = 0;
        for (dxml_t plan = dxml_child(date, "PLAN"); plan; plan = plan->next)
        {
            nPlans++;
            datumi[dateIdx].dan = dateIdx + 9;
            int planIdx = atoi(dxml_attr(plan, "NO")) - 1;

            if (planIdx < 0 || planIdx > 18)
            {
                printf("Error plan %d", planIdx + 1);
                return TimeTableErrorInvalidValue;
            }

            datumi[dateIdx].trojka[planIdx].broj_plana = GET_TXT_VALUE_FROM_CHILD(plan, "plan_number");
            datumi[dateIdx].trojka[planIdx].sat = GET_TXT_VALUE_FROM_CHILD(plan, "hour");
            datumi[dateIdx].trojka[planIdx].minut = GET_TXT_VALUE_FROM_CHILD(plan, "minut");
        }
        if (nPlans != datumi[dateIdx].broj_planova)
        {
            printf("Error on date %d invalid amount of plans.\n", dateIdx + 9);
            return TimeTableErrorInvalidValue;
        }
    }

    return TimeTableNoErrors;
}

int load_time_table(dxml_t file, stdan_t *dani, stpraznik_t *praznik, stdatum_t *datumi)
{
    dxml_t timeTable = dxml_child(file, "TIME_TABLE"); // Representation of time table in xml

    if (timeTable == NULL)
    {
        printf("Failed to load time table object from file!\n");
        return TimeTableErrorInvalidFile;
    }

    if (dani == NULL)
    {
        printf("You must provide pointer for dani!\n");
        return TimeTableErrorInvalidDays;
    }

    printf("Loading days... ");
    int err = load_days(timeTable, dani);
    if (err != TimeTableNoErrors)
    {
        printf("Error occured while loading Days.\n");
        return err;
    }
    printf("Done.\n");

    printf("Loading hollydays... ");
    err = load_hollydays(timeTable, praznik);
    if (err != TimeTableNoErrors)
    {
        printf("Error occured while loading Hollydays.\n");
        return err;
    }
    printf("Done.\n");

    printf("Loading dates... ");
    err = load_dates(timeTable, datumi);
    if (err != TimeTableNoErrors)
    {
        printf("Error occured while loading dates.\n");
        return err;
    }
    printf("Done!\n");    

    return TimeTableNoErrors;
}


int load(ssplan_t* planovi, stdan_t *dani, stpraznik_t *praznik, stdatum_t *datumi, const char* filename)
{
    auto fp = fopen(filename, "r");
    auto xml = dxml_parse_fp(fp);

    auto err = load_plans(planovi, xml);
    if (err) {
        printf("Failed to load plans: %d\n\r", err);
        // return err;
    }

    err = load_time_table(xml, dani, praznik, datumi);
    if (err) {
        printf("Failed to load time table: %d\n\r", err);
        return err;
    }

    return 0;
}

static void save_sigr(dxml_t xml, const ssplan_t& plan, int current)
{
    auto sigr = create_tag_with_attr(xml, "SIGR", "NO", current + 1);
    create_tag_with_txt(sigr, "name1", strdup("somename1"));
    create_tag_with_txt(sigr, "name2", strdup("somename2"));
    create_tag_with_txt(sigr, "enable", 0);

    for (int i = 0; i < NOSIGPAR; ++i)
    {
        char *start = new char[7];
        start[6] = 0;
        char *stop = new char[6];
        stop[5] = 0;

        snprintf(start, 7, "start%d", i + 1);
        snprintf(stop, 6, "stop%d", i + 1);

        create_tag_with_txt(sigr, start, plan.Sgrupe[current].Spar[i].start);
        create_tag_with_txt(sigr, stop, plan.Sgrupe[current].Spar[i].stop);
    }
}

static void save_stop_point(dxml_t xml, const ssplan_t &plan, uchar current)
{
    auto stop_point = create_tag_with_attr(xml, "STOP_POINT", "NO", current + 1);

    create_tag_with_txt(stop_point, "start", plan.stop_tacke[current].start);
    create_tag_with_txt(stop_point, "max", plan.stop_tacke[current].max);
    create_tag_with_txt(stop_point, "skok", plan.stop_tacke[current].skok);
}

static void save_signal_plans(dxml_t xml, const ssplan_t planovi[NOPLANS], int current)
{
    if (current == NOPLANS)
        return;
    
    auto& plan = planovi[current];

    auto sig_plan = create_tag_with_attr(xml, "SIGNAL_PLAN", "NO", current + 1);

    create_tag_with_txt(sig_plan, "RBC", plan.RBC);
    create_tag_with_txt(sig_plan, "TPP", plan.TPP);
    create_tag_with_txt(sig_plan, "REF", plan.REF);

    for (int i = 0; i < NOSIGGR; ++i)
        save_sigr(sig_plan, plan, i);

    auto stop_points = dxml_add_child(sig_plan, "STOP_POINTS", 0);
    create_tag_with_txt(stop_points, "NUMBER_OF_STOP_POINTS", plan.no_stop_tacaka);

    for (uchar i = 0; i < plan.no_stop_tacaka; ++i)
        save_stop_point(stop_points, plan, i);
    
    // </STOP_POINTS>

    create_tag_with_txt(sig_plan, "SIGNAL_PLAN_CRC", plan.CRC_plana);

    // </SIGNAL_PLAN>

    save_signal_plans(xml, planovi, current + 1);
}

void save_day(dxml_t xml, const stdan_t& dan, uchar current)
{
    dxml_t day = create_tag_with_attr(xml, "DAY", "NO", current);

    create_tag_with_txt(day, "NUMBER_OF_PLANS_PER_DAY", dan.broj_planova);

    for (int i = 0; i < dan.broj_planova; ++i)
    {
        dxml_t plan = create_tag_with_attr(day, "PLAN", "NO", i + 1);

        const auto& info = dan.trojka[i];

        create_tag_with_txt(plan, "hour", info.sat);
        create_tag_with_txt(plan, "minut", info.minut);
        create_tag_with_txt(plan, "plan_number", info.broj_plana);
    }

}

void save_date_number(dxml_t xml, const stdatum_t &datum, int current)
{
    auto date_number = create_tag_with_attr(xml, "DATE_NUMBER", "NO", current);

    create_tag_with_txt(date_number, "NUMBER_OF_PLANS_PER_DATE", datum.broj_planova);
    create_tag_with_txt(date_number, "DATE", datum.datum);
    create_tag_with_txt(date_number, "MONTH", datum.mesec);
    create_tag_with_txt(date_number, "YEAR", datum.godina);

    for (int i = 0; i < datum.broj_planova; ++i)
    {
        dxml_t plan = create_tag_with_attr(date_number, "PLAN", "NO", i + 1);

        create_tag_with_txt(plan, "hour", datum.trojka[i].sat);
        create_tag_with_txt(plan, "minut", datum.trojka[i].minut);
        create_tag_with_txt(plan, "plan_number", datum.trojka[i].broj_plana);
    }
}

void save(const ssplan_t planovi[NOPLANS], const stdan_t *dani, 
          const stpraznik_t *praznik, stdatum_t *datumi, const char* filename)
{
    auto xml = dxml_new("PROGRAM_DATA");

    auto sig_plans = dxml_add_child(xml, "SIGNAL_PLANS", 0);

    create_tag_with_txt(sig_plans, "NUMBER_OF_SIGNAL_PLANS", NOPLANS);

    save_signal_plans(sig_plans, planovi, 0);

    // </SIGNAL_PLANS>

    auto time_table = dxml_add_child(xml, "TIME_TABLE", 0);

    auto tt_day = dxml_add_child(time_table, "TIME_TABLE_DAY", 0);

    for (int i = 0; i < 7; ++i)
        save_day(tt_day, dani[i], dani[i].dan);

    // </TIME_TABLE_DAY>

    auto tt_hollyday = dxml_add_child(time_table, "TIME_TABLE_HOLLYDAY", 0);

    create_tag_with_txt(tt_hollyday, "NUMBER_OF_HOLDAYS", praznik->broj_praznika);

    for (int i = 0; i < praznik->broj_praznika; ++i)
    {
        auto doh = create_tag_with_attr(tt_hollyday, "DATE_OF_HOLIDAY", "NO", i + 1);
        create_tag_with_txt(doh, "datum", praznik->datum[i].datum);
        create_tag_with_txt(doh, "mesec", praznik->datum[i].mesec);
    }

    create_tag_with_txt(tt_hollyday, "NUMBER_OF_PLANS_PER_HOLIDAY", BROJ_PLANOVA_NA_PRAZNIKU);

    for (int i = 0; i < BROJ_PLANOVA_NA_PRAZNIKU; ++i)
    {
        dxml_t plan = create_tag_with_attr(tt_hollyday, "PLAN", "NO", i + 1);
        create_tag_with_txt(plan, "hour", praznik->planovi[i].sat);
        create_tag_with_txt(plan, "minut", praznik->planovi[i].minut);
        create_tag_with_txt(plan, "plan_number", praznik->planovi[i].broj_plana);
    }

    // </TIME_TABLE_HOLLYDAY>

    dxml_t tt_date = dxml_add_child(time_table, "TIME_TABLE_DATE", 0);

    for (int i = 9; i < 16; ++i)
        save_date_number(tt_date, datumi[i - 9], i);

    // </TIME_TABLE_DATE>

    // </TIME_TABLE>

    auto fp = fopen(filename, "w");
    auto buff = dxml_toxml(xml);
    fwrite(buff, sizeof (char), strlen(buff), fp);
    free(buff);
    fclose(fp);

}

} // namespace kss2_program
