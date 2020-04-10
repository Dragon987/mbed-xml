#include "kss2-config.h"
#include "common.h"

enum ConfigDataErrors
{
    ConfigDataNoErrors,
    ConfigDataInvalidValue,
    ConfigDataBadChildName
};

typedef unsigned char uchar;

static const char *FindInStr(const char *str, char charToFind, uchar start)
{
    int strLen = strlen(str);
    for (uchar i = start; i < strLen; i++)
        if (str[i] == charToFind)
            return &str[i];
    return str;
}

// U msg se ubacuje string koji se formatira na isti nacin kao printf sa formatVals argumentima
#define KSS_ASSERT(expr, errorCode, msg, formatVals...) \
    if (!(expr))                                        \
    {                                                   \
        printf(msg, formatVals);                        \
        return errorCode;                               \
    }

// Takodje pravi i novu dxml_t varijablu sa onim imenom koje stoji na child
#define CHECK_IF_CHILD_EXISTS(parent, child, childName) \
    dxml_t child = dxml_child(parent, childName);       \
    KSS_ASSERT(child, ConfigDataBadChildName, "Child with name %s does not exist\n", childName)
// INC stands for inclusive
#define VALUE_IN_RANGE_INC(val, min, max) (val >= min && val <= max)

#define FILL_VALUE(parent, child, childName, dest, errCode, min, max, msg, formats...) \
    CHECK_IF_CHILD_EXISTS(parent, child, childName);                                   \
    dest = atoi(child->txt);                                                           \
    KSS_ASSERT(VALUE_IN_RANGE_INC(atoi(child->txt), min, max), errCode, msg, formats)

#define FILL_VALUE_NO_ASSERT(parent, child, childName, dest) \
    CHECK_IF_CHILD_EXISTS(parent, child, childName);         \
    dest = atoi(child->txt)


namespace kss2_config
{

auto load(const char *fileName,
          EEfolder *ef,
          SSPIfolder *sspi,
          SIOSijl *ios,
          SConflP *cc,
          uchar *mg) -> int
{
    auto fp = fopen(fileName, "r");
    dxml_t file = dxml_parse_fp(fp);
    CHECK_IF_CHILD_EXISTS(file, controlerConfig, "CONTROLLER_CONFIG");

    // for (dxml_t c = controlerConfig->child; c; c = c->sibling)
    //     printf("%s\n", c->name);

    CHECK_IF_CHILD_EXISTS(controlerConfig, eef, "EEfolder");

    CHECK_IF_CHILD_EXISTS(eef, sg, "nr_sg");
    ef->noSigGr = atoi(sg->txt);
    KSS_ASSERT(ef->noSigGr >= NO_SIGR_MIN && ef->noSigGr <= NO_SIGR_MAX, ConfigDataInvalidValue,
               "Broj SG nije dobar u EEFolderu, vrednost je %d.\n", ef->noSigGr)

    CHECK_IF_CHILD_EXISTS(eef, noIO, "no_IO");
    ef->noIOadr = atoi(noIO->txt);
    KSS_ASSERT(ef->noIOadr >= NO_IO_MIN && ef->noIOadr <= NO_IO_MAX, ConfigDataInvalidValue,
               "Broj IO nije dobar u EEFolderu, vrednost je %d.\n", ef->noIOadr)

    CHECK_IF_CHILD_EXISTS(eef, noCFP, "no_CFP");
    ef->noCflPr = atoi(noCFP->txt);

    CHECK_IF_CHILD_EXISTS(eef, noFB, "no_FB");
    ef->noCrkSij = atoi(noFB->txt);
    KSS_ASSERT(ef->noCrkSij >= NO_PREGORELIH_SIJALICA_MIN && ef->noCrkSij <= NO_PREGORELIH_SIJALICA_MAX,
               ConfigDataInvalidValue,
               "Broj pregorelih sijalica nije dobar, on je %d\n",
               ef->noCrkSij);

    CHECK_IF_CHILD_EXISTS(eef, gtCRC, "mGT_CRC");
    ef->mGreenTblCRC = atoi(gtCRC->txt);

    CHECK_IF_CHILD_EXISTS(eef, cft, "CFT_CRC");
    ef->ConflPTblCRC = atoi(cft->txt);

    CHECK_IF_CHILD_EXISTS(eef, iot, "IOT_CRC");
    ef->SiGrIOTblCRC = atoi(iot->txt);

    CHECK_IF_CHILD_EXISTS(eef, eeAddr, "EEAdr");

    const char *txt = eeAddr->txt;

    int lastComma = 0;

    SEE_Adrese *adresa = &(ef->adrese);

    // Posto su svi clanovi SEE_Adrese uchar onda je najveca vrednost 255 a broj cifara 3
    char number[3];

    // Ovo resenje veoma zavisi od rasporeda clanova u strukturi SEE_Adrese u ovom trenutku
    for (int i = 0; i < 4; i++)
    {
        const char *comma = FindInStr(txt, ',', lastComma);
        int count = 0;

        int newComma = comma - txt;
        if (newComma == 0)
            newComma = strlen(txt);

        number[2] = ' ';
        for (int j = lastComma; j < newComma; j++)
            if (txt[j] != ' ')
                number[count++] = txt[j];

        // printf("%d ", lastComma);
        lastComma = newComma + 1;

        *(((uchar *)(adresa)) + i) = atoi(number);
        // printf("%d\n", *(((uchar *)(adresa)) + i));
    }

    CHECK_IF_CHILD_EXISTS(controlerConfig, sigGroups, "SIGNAL_GROUPS");
    CHECK_IF_CHILD_EXISTS(sigGroups, nvsg, "nr_vsg");
    sspi->brvg = atoi(nvsg->txt);
    KSS_ASSERT(VALUE_IN_RANGE_INC(sspi->brvg, NO_SPEC_SG_MIN, NO_SPEC_SG_MAX),
               ConfigDataInvalidValue,
               "Losa vrednost za broj vozackih sg, %d\n", sspi->brvg);

    CHECK_IF_CHILD_EXISTS(sigGroups, ntsg, "nr_tsg");
    sspi->brtg = atoi(ntsg->txt);
    KSS_ASSERT(VALUE_IN_RANGE_INC(sspi->brtg, NO_SPEC_SG_MIN, NO_SPEC_SG_MAX),
               ConfigDataInvalidValue,
               "Losa vrednost za broj tramvajskih sg, %d\n", sspi->brtg);

    CHECK_IF_CHILD_EXISTS(sigGroups, npsg, "nr_psg");
    sspi->brpg = atoi(npsg->txt);
    KSS_ASSERT(VALUE_IN_RANGE_INC(sspi->brpg, NO_SPEC_SG_MIN, NO_SPEC_SG_MAX),
               ConfigDataInvalidValue,
               "Losa vrednost za broj pesackih sg, %d\n", sspi->brpg);

    CHECK_IF_CHILD_EXISTS(sigGroups, nasg, "nr_asg");
    sspi->brstrel = atoi(nasg->txt);
    KSS_ASSERT(VALUE_IN_RANGE_INC(sspi->brstrel, NO_SPEC_SG_MIN, NO_SPEC_SG_MAX),
               ConfigDataInvalidValue,
               "Losa vrednost za broj sg strelica, %d\n",
               sspi->brstrel);

    CHECK_IF_CHILD_EXISTS(controlerConfig,
                          transTimes,
                          "TRANSIENT_TIMES");

    CHECK_IF_CHILD_EXISTS(transTimes,
                          ryTime,
                          "ry_t");
    sspi->czuto_v = atoi(ryTime->txt);
    KSS_ASSERT(VALUE_IN_RANGE_INC(sspi->czuto_v,
                                  RED_YELLOW_TIME_MIN,
                                  RED_YELLOW_TIME_MAX),
               ConfigDataInvalidValue,
               "Bad value for red yellow times %d\n", sspi->czuto_v);

    CHECK_IF_CHILD_EXISTS(transTimes, yTime, "y_t");
    sspi->zuto_v = atoi(yTime->txt);
    KSS_ASSERT(VALUE_IN_RANGE_INC(
                   sspi->zuto_v,
                   YELLOW_TIME_MIN,
                   YELLOW_TIME_MAX),
               ConfigDataInvalidValue,
               "Invalid value for yellow times %d\n",
               sspi->zuto_v);

    CHECK_IF_CHILD_EXISTS(controlerConfig,
                          startTimes,
                          "START_TIMES");

    FILL_VALUE(startTimes,
               yfl,
               "yfl",
               sspi->start_times.zuti_treptac,
               ConfigDataInvalidValue,
               YELLOW_FLASH_TIME_MIN,
               YELLOW_FLASH_TIME_MAX,
               "Bad value for yellow flash time %d\n",
               sspi->start_times.zuti_treptac);

    FILL_VALUE(startTimes, allYellow, "y",
               sspi->start_times.sve_zuto,
               ConfigDataInvalidValue,
               ALL_YELLOW_TIME_MIN,
               ALL_YELLOW_TIME_MAX,
               "Bad value for yellow time %d\n",
               sspi->start_times.sve_zuto);

    FILL_VALUE(startTimes, allRed, "ar",
               sspi->start_times.sve_crveno,
               ConfigDataInvalidValue,
               ALL_RED_TIME_MIN,
               ALL_RED_TIME_MAX,
               "Bad all red time %d\n",
               sspi->start_times.sve_crveno);

    lastComma = 0;
    CHECK_IF_CHILD_EXISTS(controlerConfig,
                          pNP, "PLUG_AND_PLAY");
    CHECK_IF_CHILD_EXISTS(pNP, pNPData, "data");
    txt = pNPData->txt;

    for (int i = 0; i < PNP_VEL; i++)
    {
        for (int j = 0; j < 3; j++)
            number[j] = ' ';

        const char *comma = FindInStr(txt, ',', lastComma);
        int newComma = comma - txt;
        if (newComma == 0)
            newComma = strlen(txt);

        int count = 0;
        for (int j = lastComma; j < newComma; j++)
            if (txt[j] != ' ')
                number[count++] = txt[j];

        lastComma = newComma + 1;

        sspi->PnP[i] = atoi(number);
    }

    FILL_VALUE_NO_ASSERT(controlerConfig,
                         recPerMin,
                         "REC_PER_MIN",
                         sspi->zapis_na_minuta);

    FILL_VALUE(controlerConfig,
               fPlanNum,
               "FPLAN_NUMBER",
               sspi->fors_plan,
               ConfigDataInvalidValue,
               FPLAN_NUMBER_MIN,
               FPLAN_NUMBER_MAX,
               "Bad value for fPlanNumber %d\n",
               sspi->fors_plan);

    FILL_VALUE_NO_ASSERT(controlerConfig, noSG,
                         "NROF_SIG_PLANS",
                         sspi->broj_planova);

    FILL_VALUE_NO_ASSERT(controlerConfig,
                         corrFlag,
                         "CORRECT_FLAG",
                         sspi->broj_planova);

    CHECK_IF_CHILD_EXISTS(controlerConfig,
                          timeTableCRC,
                          "TIME_TABLE_CRC");

    FILL_VALUE_NO_ASSERT(timeTableCRC,
                         day, "day",
                         sspi->CRC_dana);

    FILL_VALUE_NO_ASSERT(timeTableCRC,
                         holday, "holday",
                         sspi->CRC_praznika);

    CHECK_IF_CHILD_EXISTS(controlerConfig,
                          bios,
                          "BIOS");

    FILL_VALUE(bios, mod, "mod",
               sspi->Bios.proc.mod,
               ConfigDataInvalidValue,
               0, 3,
               "Bad value for mod %d\n",
               atoi(mod->txt));

    FILL_VALUE(bios, cordLoc, "cord_loc",
               sspi->Bios.proc.koord_lok,
               ConfigDataInvalidValue,
               0, 1,
               "Bad value for cord loc %d\n",
               atoi(cordLoc->txt));

    FILL_VALUE(bios, forceTable, "fors_table",
               sspi->Bios.proc.fors_tab,
               ConfigDataInvalidValue,
               0, 1,
               "Bad value for fors tab %d\n",
               atoi(forceTable->txt));

    FILL_VALUE(bios, vh, "vh_fl",
               sspi->Bios.proc.v_trep,
               ConfigDataInvalidValue,
               0, 1,
               "Bad value for vh_fl %d\n",
               atoi(vh->txt));

    FILL_VALUE(bios, pd, "pd_fl",
               sspi->Bios.proc.p_trep,
               ConfigDataInvalidValue,
               0, 1,
               "Bad value for pd_fl %d\n",
               atoi(pd->txt));

    FILL_VALUE(bios, adaptive, "adaptive",
               sspi->Bios.proc.detektor,
               ConfigDataInvalidValue,
               0, 1,
               "Bad value for adaptive %d\n",
               atoi(adaptive->txt));

    FILL_VALUE(bios, stages, "stages",
               sspi->Bios.proc.najava,
               ConfigDataInvalidValue,
               0, 1,
               "Bad value for stages %d\n",
               atoi(stages->txt));

    CHECK_IF_CHILD_EXISTS(controlerConfig,
                          restart, "RESART");

    FILL_VALUE_NO_ASSERT(restart, noRestarts,
                         "nrof_restarts",
                         sspi->Restart.no);

    FILL_VALUE_NO_ASSERT(restart, waitForRestart,
                         "wait_for_restart",
                         sspi->Restart.time);

    CHECK_IF_CHILD_EXISTS(controlerConfig,
                          lastError, "LAST_ERROR");

    // for (dxml_t c = lastError->child; c; c = c->sibling)
    //     printf("%s\n", c->name);

    CHECK_IF_CHILD_EXISTS(lastError,
                          lastErrorDrop,
                          "LAST_ERROR_DROP");

    CHECK_IF_CHILD_EXISTS(lastErrorDrop, error, "ERROR");
    FILL_VALUE_NO_ASSERT(error,
                         type,
                         "type",
                         sspi->ErrorTimeComm.sdrop.serror.type);

    FILL_VALUE_NO_ASSERT(error,
                         addr,
                         "address",
                         sspi->ErrorTimeComm.sdrop.serror.address);

    FILL_VALUE_NO_ASSERT(error,
                         data,
                         "data",
                         sspi->ErrorTimeComm.sdrop.serror.data);

    FILL_VALUE_NO_ASSERT(lastErrorDrop,
                         planNo,
                         "PLAN_NUMBER",
                         sspi->ErrorTimeComm.sdrop.broj_plana);

    FILL_VALUE_NO_ASSERT(lastErrorDrop,
                         cycSec,
                         "CYC_SEC",
                         sspi->ErrorTimeComm.sdrop.vreme_u_ciklusu);

    CHECK_IF_CHILD_EXISTS(lastErrorDrop,
                          errorRealTime,
                          "ERROR_REAL_TIME");

    FILL_VALUE_NO_ASSERT(errorRealTime,
                         hour, "hour",
                         sspi->ErrorTimeComm.sdrop.realtime.hour);

    FILL_VALUE_NO_ASSERT(errorRealTime,
                         min, "min",
                         sspi->ErrorTimeComm.sdrop.realtime.min);

    FILL_VALUE_NO_ASSERT(errorRealTime,
                         sec, "sec",
                         sspi->ErrorTimeComm.sdrop.realtime.sec);

    FILL_VALUE_NO_ASSERT(errorRealTime,
                         date, "date",
                         sspi->ErrorTimeComm.sdrop.realtime.date);

    FILL_VALUE_NO_ASSERT(errorRealTime,
                         month, "month",
                         sspi->ErrorTimeComm.sdrop.realtime.month);

    FILL_VALUE_NO_ASSERT(errorRealTime,
                         year, "year",
                         sspi->ErrorTimeComm.sdrop.realtime.year);

    CHECK_IF_CHILD_EXISTS(lastError,
                          lastErrorNoDrop,
                          "LAST_ERROR_NONDROP");

    CHECK_IF_CHILD_EXISTS(lastErrorNoDrop,
                          error2,
                          "ERROR");

    // Extracting values from string
    // This solution assumes that values are in order:
    // 1) type
    // 2) address
    // 3) data

    txt = error2->txt;
    lastComma = 0;
    for (int i = 0; i < 3; i++) // 3 because there are 3 values in struct
    {
        for (int j = 0; j < 3; j++)
            number[j] = ' ';

        const char *comma = FindInStr(txt, ',', lastComma);

        int newComma = comma - txt;
        if (newComma == 0)
            newComma = strlen(txt);

        int count = 0;
        for (int j = lastComma; j < newComma; j++)
            if (txt[j] != ' ')
                number[count++] = txt[j];

        lastComma = newComma + 1;
        // printf("%d\n", atoi(number));
        *((uchar *)(&(sspi->ErrorTimeComm.snondrop.serror)) + i) = atoi(number);
    }

    FILL_VALUE_NO_ASSERT(lastErrorNoDrop,
                         planNum2,
                         "PLAN_NUMBER",
                         sspi->ErrorTimeComm.snondrop.broj_plana);

    FILL_VALUE_NO_ASSERT(lastErrorNoDrop,
                         cycSec2, "CYC_SEC",
                         sspi->ErrorTimeComm.snondrop.vreme_u_ciklusu);

    CHECK_IF_CHILD_EXISTS(lastErrorNoDrop, errRT, "ERROR_REAL_TIME");

    SRealTime_ *sspiRealTime = &(sspi->ErrorTimeComm.snondrop.realtime);

    FILL_VALUE_NO_ASSERT(errRT, hour2, "hour", sspiRealTime->hour);
    FILL_VALUE_NO_ASSERT(errRT, min2, "min", sspiRealTime->min);
    FILL_VALUE_NO_ASSERT(errRT, sec2, "sec", sspiRealTime->sec);
    FILL_VALUE_NO_ASSERT(errRT, date2, "date", sspiRealTime->date);
    FILL_VALUE_NO_ASSERT(errRT, month2, "month", sspiRealTime->month);
    FILL_VALUE_NO_ASSERT(errRT, year2, "year", sspiRealTime->year);

    CHECK_IF_CHILD_EXISTS(lastError, lastErrorFlags, "LAST_ERROR_FLAGS");

    SDrerr_byte *errFlags = &sspi->ErrorTimeComm.sflags;

    FILL_VALUE_NO_ASSERT(lastErrorFlags, err1, "drerr_pg1", errFlags->drerr_pg1);
    FILL_VALUE_NO_ASSERT(lastErrorFlags, err2, "drerr_pg2", errFlags->drerr_pg2);
    FILL_VALUE_NO_ASSERT(lastErrorFlags, err3, "drerr_pg3", errFlags->drerr_pg3);
    FILL_VALUE_NO_ASSERT(lastErrorFlags, err4, "drerr_pg4", errFlags->drerr_pg4);
    FILL_VALUE_NO_ASSERT(lastErrorFlags, dropFlag, "drop_flag", errFlags->drop);
    FILL_VALUE_NO_ASSERT(lastErrorFlags, noDropFlag, "nondrop_flag", errFlags->nondrop);
    FILL_VALUE_NO_ASSERT(lastErrorFlags, cnflSwitch, "CNFLBoard_mod_switch", errFlags->prekidac);

    CHECK_IF_CHILD_EXISTS(controlerConfig, errBufferCtrl, "ERROR_BUFFER_CONTROL");

    FILL_VALUE(errBufferCtrl, lastErrorInBuffer, "LAST_ERROR_IN_BUFFER", sspi->ErrorCtrl.brojac_buff,
               ConfigDataInvalidValue, LAST_ERROR_IN_BUFFER_MIN, LAST_ERROR_IN_BUFFER_MAX,
               "Bad value for last error in buffer %d\n", atoi(lastErrorInBuffer->txt));

    CHECK_IF_CHILD_EXISTS(errBufferCtrl, IDLE, "IDLE");

    CHECK_IF_CHILD_EXISTS(IDLE, idle, "idle");
    int attr = atoi(dxml_attr(idle, "NO"));
    if (attr == 1)
        sspi->ErrorCtrl.idle = atoi(idle->txt);
    else if (attr == 2)
        sspi->ErrorCtrl.idle1 = atoi(idle->txt);
    else
    {
        printf("Invalid idle number\n");
        return ConfigDataInvalidValue;
    }

    idle = idle->next;
    if (!idle)
    {
        printf("There are not 2 idles\n");
        return ConfigDataBadChildName;
    }

    int attr2 = atoi(dxml_attr(idle, "NO"));
    if (attr == attr2)
    {
        printf("idles have same NO attribute! %d\n", attr);
        return ConfigDataInvalidValue;
    }
    attr = attr2;
    if (attr == 1)
        sspi->ErrorCtrl.idle = atoi(idle->txt);
    else if (attr == 2)
        sspi->ErrorCtrl.idle1 = atoi(idle->txt);
    else
    {
        printf("Invalid idle number\n");
        return ConfigDataInvalidValue;
    }

    FILL_VALUE_NO_ASSERT(controlerConfig, letoZima, "SUMMER_WINTER", sspi->leto_zima);
    FILL_VALUE_NO_ASSERT(controlerConfig, none, "NONE", sspi->none);

    CHECK_IF_CHILD_EXISTS(file, cnflConfig, "CNFLBoard_CONFIG");

    CHECK_IF_CHILD_EXISTS(cnflConfig, ioModules, "IO_MODULES_TABLE");

    int numberIO;
    FILL_VALUE(ioModules, noIOMods, "NUMBER_OF_IO_MODULES",
               numberIO, ConfigDataInvalidValue, NO_IO_MIN, NO_IO_MAX,
               "Bad number of io modules %d\n", atoi(noIOMods->txt));

    // First io module
    CHECK_IF_CHILD_EXISTS(ioModules, io, "IO");

    int lastIOIdx = atoi(dxml_attr(io, "NO"));
    if (!VALUE_IN_RANGE_INC(lastIOIdx, NO_IO_MIN, numberIO))
    {
        printf("Bad io number %d\n", lastIOIdx);
        return ConfigDataInvalidValue;
    }
    lastIOIdx--;

    for (dxml_t out = dxml_child(io, "OUT"); out; out = out->next)
    {
        int outIdx = atoi(dxml_attr(out, "NO")) - 1;
        SSijlelem *adr = &ios[lastIOIdx].sijlelem[outIdx];
        FILL_VALUE_NO_ASSERT(out, sub, "sub", adr->sub);
        FILL_VALUE_NO_ASSERT(out, sigr, "sigr", adr->sigr);
        FILL_VALUE_NO_ASSERT(out, valid, "valid", adr->valid);
        FILL_VALUE_NO_ASSERT(out, collor, "collor", adr->collor);
        FILL_VALUE_NO_ASSERT(out, led, "led", adr->led);
        FILL_VALUE_NO_ASSERT(out, osig, "osig", adr->osig);
        FILL_VALUE_NO_ASSERT(out, yell, "yell", adr->yell);
        FILL_VALUE_NO_ASSERT(out, sig, "sig", adr->sign);
    }

    for (io = io->next; io; io = io->next)
    {
        int ioIdx = atoi(dxml_attr(io, "NO"));
        if (!(VALUE_IN_RANGE_INC(ioIdx, NO_IO_MIN, numberIO)) || ioIdx != lastIOIdx + 2)
        {
            printf("Bad io number %d\n", ioIdx);
            return ConfigDataInvalidValue;
        }
        ioIdx--;
        lastIOIdx = ioIdx;

        for (dxml_t out = dxml_child(io, "OUT"); out; out = out->next)
        {
            int outIdx = atoi(dxml_attr(out, "NO")) - 1;
            SSijlelem *adr = &ios[lastIOIdx].sijlelem[outIdx];
            FILL_VALUE_NO_ASSERT(out, sub, "sub", adr->sub);
            FILL_VALUE_NO_ASSERT(out, sigr, "sigr", adr->sigr);
            FILL_VALUE_NO_ASSERT(out, valid, "valid", adr->valid);
            FILL_VALUE_NO_ASSERT(out, collor, "collor", adr->collor);
            FILL_VALUE_NO_ASSERT(out, led, "led", adr->led);
            FILL_VALUE_NO_ASSERT(out, osig, "osig", adr->osig);
            FILL_VALUE_NO_ASSERT(out, yell, "yell", adr->yell);
            FILL_VALUE_NO_ASSERT(out, sig, "sig", adr->sign);
        }
    }

    // Conflict couples

    CHECK_IF_CHILD_EXISTS(cnflConfig, conCouples, "CONFLICT_COUPLES");

    int noConCoup;
    FILL_VALUE(conCouples, noCon, "NUMBER_OF_CONFLICT_COUPLES",
               noConCoup, ConfigDataInvalidValue,
               NO_CONFL_COUPLES_MIN,
               NO_CONFL_COUPLES_MAX,
               "Bad number of confl couples %d\n",
               noConCoup);

    short first = 1;
    int lastConIdx, currConIdx;
    for (dxml_t conCoup = dxml_child(conCouples, "CONFLICT_COUPLE");
         conCoup; conCoup = conCoup->next)
    {
        if (first == 1)
        {
            lastConIdx = atoi(dxml_attr(conCoup, "N")) - 1;
            currConIdx = lastConIdx;
        }
        else
            currConIdx = atoi(dxml_attr(conCoup, "N")) - 1;

        if (!(VALUE_IN_RANGE_INC(currConIdx, NO_CONFL_COUPLES_MIN - 1,
                                 noConCoup - 1)) ||
            (first != 1 && currConIdx != lastConIdx + 1))
        {
            printf("Bad confli couple number %d\n", currConIdx);
            return ConfigDataInvalidValue;
        }
        first = 0;

        SConflP *c = &(cc[currConIdx]);
        FILL_VALUE(conCoup, sig, "signal_group_01", c->sigr1,
                   ConfigDataInvalidValue, NO_SIGR_MIN, NO_SIGR_MAX,
                   "Bad sg number %d\n", c->sigr1);
        FILL_VALUE(conCoup, signalGroup, "signal_group_02", c->sigr2,
                   ConfigDataInvalidValue, NO_SIGR_MIN, NO_SIGR_MAX,
                   "Bad sg number %d\n", c->sigr2);

        FILL_VALUE_NO_ASSERT(conCoup, ftime, "ftime", c->ftime);
        FILL_VALUE_NO_ASSERT(conCoup, ltime, "ltime", c->ltime);

        lastConIdx = currConIdx;
    }

    CHECK_IF_CHILD_EXISTS(cnflConfig, minGreenTimes, "MINGREEN_TIMES")

    int noMG;
    FILL_VALUE_NO_ASSERT(minGreenTimes, nomingreen,
                         "NUMBER_OF_MINGREEN_TIMES",
                         noMG);

    dxml_t mingreen = dxml_child(minGreenTimes, "min_green");
    for (int i = 0; i < noMG; i++)
    {
        mg[i] = atoi(mingreen->txt);
        mingreen = mingreen->next;
    }

    dxml_free(file);
    return ConfigDataNoErrors;
}

auto save(const char *fileName,
          const EEfolder *ef,
          const SSPIfolder *sspi,
          const SIOSijl *ios,
          const SConflP *cc,
          const uchar *mg) -> int
{
    return 0;
}
} // namespace kss2_config