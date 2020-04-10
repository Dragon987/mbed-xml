#pragma once

#define BR_FAZA 8
#define BR_PRELAZA 16
#define BR_PIFOVA 8
#define BR_FAZA_uPIFu 8
#define BR_TABELA 16
#define BR_CVOROVA_AF 32
#define BR_CVOROVA_AP 32
#define BR_ALG_FAZA BR_FAZA
#define BR_ALG_PRELAZA 32
#define CNST 16
#define BR_DETEKTORA 32
#define BR_RAM_LOKACIJA (3 * CNST + BR_DETEKTORA + BR_DETEKTORA + BR_DETEKTORA) /*  sistemske[16]+radne[32]+detektori[32]+zauzece[32]+gap[32]	*/
#define BR_batRAM_LOKACIJA (BR_TABELA * 2 * CNST - 1)
#define BR_ALG_FAZA BR_FAZA
#define BR_F_CVOROVA 24
#define BR_BOJA 32

//#define BR_ALG_PRELAZA		 4
#define BR_FP_CVOROVA 24

#define PROG_DET_MARKER 0xe5
#define PROG_toCD_MARKER 0xe6

#define GET_DET_STATE 0x41  /*  zapis u flash na svakih 5 min */
#define SET_TIME_ZAPIS 0x42 /*  citanje 5 minutnog zapisa   */
#define DET_kaPROG 0x43     /*  detektori ka programskoj */
#define SET_CYC_ZAPIS 0x44  /*  citanje 5 minutnog zapisa   */
#define GET_ZAPIS_SAUS 0x45 /*  citanje 5 minutnog zapisa   */

#define SET_DETCD_toCD 0x6a /*  marker za slanje stanja brojacima   */

using uchar = unsigned char;
using byte = uchar;
using schar = signed char;
using uint = unsigned int;

namespace kss2_adaptiv
{
struct SFaza /*  max broj je 8 */
{
  uchar broj;  /*  broj faze */
  uchar min_h; /*  minimalno vreme boravka u fazi  */
  uchar min_l; /*  minimalno vreme boravka u fazi  */
  uchar opt_h; /*  optimalno vreme boravka u fazi  */
  uchar opt_l; /*  optimalno vreme boravka u fazi  */
  uchar max_h; /*  maksimalno vreme boravka u fazi */
  uchar max_l; /*  maksimalno vreme boravka u fazi */
  uchar boje[4];
  /*  !!!!   ovo bi trebao biti "long"  -  jedan bit -> boja jedne signalne grupe !!!! */
  byte CRC_faze_h;
  byte CRC_faze_l;
};

struct SPrelaz /*  max broj je 16 ?????????  */
{
  uchar broj_sadasnje;
  uchar broj_naredne;
  uchar duzina;
  uchar toggle[32]; /* ???!!!???*/ /*  sekunda promene 'boje' */
  uchar CRC_prelaza_h;
  uchar CRC_prelaza_l;
};

struct SParametriFaze
{
  uchar broj;  /*  broj faze */
  uchar opt_h; /*  optimalno vreme boravka u fazi */
  uchar opt_l; /*  optimalno vreme boravka u fazi */
};

struct SPlanIzmenaFaza /*  max broj je 16 - vezuje se za plan tempiranja signalnih planova */
{
  uchar broj_PIFa;  /*  broj plana izmena faza */
  uchar broj_faza;  /*  broj faza u nizu (ciklusu) max 8 */
  uchar duzina_cyc; /*  duzina kvazi ciklusa */
  uchar start_sec;  /*  startna sekunda */
  struct SParametriFaze faze[BR_FAZA_uPIFu];
  uchar CRC_PlanIzmena_h;
  uchar CRC_PlanIzmena_l;
};

struct DET_descr
{
  uchar detector_description[16];
};

struct SA_sledeci
{

  uint pozitivan : 5;
  uint negativan : 5;
  uint p_end : 1;
  uint n_end : 1;
  uint rezerva : 4;
};

union U_SA_sledeci {
  uchar tmp[2];
  struct SA_sledeci sledeci;
};

struct SFRincrm /* inkrementiranje RAM lokacija */
{
  uchar R1;
  uchar R2;
  uchar R3;
  uchar R4;
  uchar R5;
  uchar R6;
};

struct SFRincrm_cnt /* inkrementiranje RAM lokacija za konstantu */
{
  // sint     skok;
  schar skok_h;
  schar skok_l;
  uchar R;
};

struct SFRplus_ram /* inkrementiranje RAM lokacija za konstantu */
{
  uchar Rleft;
  uchar R1;
  uchar R2;
  uchar R3;
  uchar R4;
  uchar R5;
};

struct SFRdel /* brisanje RAM lokacija */
{
  uchar R1;
  uchar R2;
  uchar R3;
  uchar R4;
  uchar R5;
  uchar R6;
};

struct SFdogadjaj /* dogadjaji */
{
  uchar broj;
};

struct SFfaza_set /* pozivanje i setovanje naredne faze */
{
  uchar faza;
  uchar tmin;
  uchar topt;
  uchar tmax;
  uchar prelaz;
};

struct SFZTP_set /* pozivanje i setovanje naredne faze */
{
  uchar pgr_br;
};

union Udata_iffun {
  uchar data[6];
  SFdogadjaj set_dogadj; /* setovanje dogadj */
  SFdogadjaj res_dogadj; /* setovanje dogadj */
  SFRincrm increm;       /* inkrementiranje RAM lokacija */
  SFRincrm_cnt inc_cnst; /* inkrementiranje RAM lokacija za konstantu */
  SFRdel delet;          /* brisanje RAM lokacija */
  SFfaza_set set_faze;   /* pozivanje i setovanje naredne faze */
  SFZTP_set set_ztp;     /* pozivanje i setovanje naredne faze */
  SFRplus_ram rplusr;    /* inkrementiranje RAM lokacija za konstantu */
};


struct SUDopseg /* detektor u opsegu */
{
  //long det; MT
  uchar det4;
  uchar det3;
  uchar det2;
  uchar det1;
  uchar start;
  uchar stop;
};

struct SUdogadjaj /* detektor u opsegu */
{
  uchar broj;
};

struct SUCRopseg /* ram lokacija u opsegu */
{
  // uint Cnst_leva;
  // uint Cnst_desna;
  uchar Cnst_leva_h;
  uchar Cnst_leva_l;

  uchar Cnst_desna_h;
  uchar Cnst_desna_l;

  uchar R;
};

struct SURRporedj /* 'poredjenje' dve ram lokacije ( koje je poredjenje govori nam /tip.if_fun/ */
{
  uchar R_levo;
  uchar R_desno;
};

struct SURi_ili /* ili f-ja svih RAM lokacija  */
{
  uchar R1;
  uchar R2;
  uchar R3;
  uchar R4;
  uchar R5;
  uchar R6;
};

struct SURRopseg /* ram lokacija u opsegu dve ram lokacije*/
{
  uchar R_levo;
  uchar R;
  uchar R_desno;
};

union Udata_uslova {
  uchar data[6];
  SUDopseg det_uCops;   /* detektor u opsegu */
  SUdogadjaj dogadjaj;  /* dogadjaj u opsegu */
  SUCRopseg ram_uCops;  /* ram lokacija u opsegu */
  SURRopseg ram_uRops;  /* ram lokacija u opsegu dve ram lokacije*/
  SURRporedj RR_poredj; /* 'poredjenje' dve ram lokacije ( koje je poredjenje govori nam /tip.if_fun/ */
  SURi_ili Ri_ili;      /* ili f-ja svih RAM lokacija  */
};

struct SA_cvor
{
  uchar uslov;
  uchar uslov_data;
  union Udata_uslova U; // uchar  [ 6 ]
  uchar fun;
  uchar fun_data; // uchar  [ 6 ]
  union Udata_iffun F;
  union U_SA_sledeci u_SA;
};

uint *get_ram_locations();

} // namespace kss2_adaptiv