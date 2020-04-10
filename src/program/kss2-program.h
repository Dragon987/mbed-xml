#pragma once

#include "dxml.h"

#define NOPLANS 16
#define NOSIGGR         32
#define NOSTOPTACAKA    5
#define NOSIGPAR 3

#define  BROJ_DANA          7
#define  BROJ_DATUMA        8
#define  NODNEVNIHPLANOVA   19
#define BROJ_PRAZNIKA 16


typedef unsigned char uchar;
typedef unsigned int uint;

namespace kss2_program
{

struct sspar_t        { uchar start, stop; };
struct ssgrupa_t      { sspar_t Spar[NOSIGPAR]; };
struct sStopTacka_t   { uchar start, max, skok; };

struct ssplan_t
{  
    uchar broj_plana;
    uchar RBC;                   /* ciklus */        
    uchar TPP;                   /* tacka izmene programa */
    uchar REF;                   /* koristi se za offset */
    ssgrupa_t Sgrupe[NOSIGGR];                /* max: 32 sig. grupe */
    sStopTacka_t stop_tacke[NOSTOPTACAKA];    /* max: 5 stop tacaka */
    uchar no_stop_tacaka;                          /* broj koliko ih ima u konkretnom programu */
    uint CRC_plana;
};

enum SIGRErrorCodes
{
    SIGR_NO_ERROR, SIGR_NUMBER_ERROR, SIGR_START_STOP, NO_CRC
};

struct stparovi_t // Plans
{  
  uchar sat; 
  uchar minut; 
  uchar broj_plana; 
} ;

struct stdatum_t
{
  uchar dan;
  uchar broj_planova;
  uchar datum;
  uchar mesec;
  uchar godina;
  stparovi_t trojka[NODNEVNIHPLANOVA];
} ;

struct stdan_t // Day 
{  
  uchar dan;
  uchar broj_planova;
  stparovi_t trojka[NODNEVNIHPLANOVA];
} ;

struct stdatumi_t
{  
  uchar datum;
  uchar mesec;
} ;

struct stpraznik_t // Hollyday
{  
uchar broj_praznika;
stdatumi_t datum[BROJ_PRAZNIKA];
stparovi_t planovi[NODNEVNIHPLANOVA];
} ;

enum TimeTableErrors
{
  TimeTableNoErrors = 0, TimeTableErrorInvalidFile, TimeTableErrorInvalidDays,
  TimeTableErrorInvalidPlanNumber, TimeTableErrorInvalidValue
};

int load(ssplan_t* planovi, stdan_t *dani, stpraznik_t *praznik, stdatum_t *datumi, const char* filename);

}
