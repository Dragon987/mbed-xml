// /*
//  * detektor.h
//  *
//  *  Created on: Aug 15, 2015
//  *      Author: miroslavt
//  */

// #pragma once


// #define CNST	             16 
// #define	BR_DETEKTORA         32
// #define	BR_RAM_LOKACIJA      ( 3 * CNST + BR_DETEKTORA + BR_DETEKTORA + BR_DETEKTORA ) 	/*  sistemske[16]+radne[32]+detektori[32]+zauzece[32]+gap[32]	*/
// #define	BR_batRAM_LOKACIJA   ( BR_TABELA * 2 * CNST - 1 )

// #define BR_ALG_FAZA		BR_FAZA
// #define	BR_F_CVOROVA        24

// //#define BR_ALG_PRELAZA		 4
// #define	BR_FP_CVOROVA       24 


// #define  PROG_DET_MARKER   0xe5
// #define  PROG_toCD_MARKER  0xe6

// #define  GET_DET_STATE    0x41 	  /*  zapis u flash na svakih 5 min */
// #define  SET_TIME_ZAPIS   0x42 	  /*  citanje 5 minutnog zapisa   */
// #define  DET_kaPROG       0x43 	  /*  detektori ka programskoj */
// #define  SET_CYC_ZAPIS    0x44 	  /*  citanje 5 minutnog zapisa   */
// #define  GET_ZAPIS_SAUS   0x45 	  /*  citanje 5 minutnog zapisa   */

// #define  SET_DETCD_toCD   0x6a 	  /*  marker za slanje stanja brojacima   */

// struct SZapis
// {
//   uint             broj_zapisa; 
//   uchar            plan_faza; 
//   uint             brojanje[ BR_DETEKTORA ]; 
//   uchar            rezerva[ 10 ]; 
//   struct casovnik  time;
// };	

// /*  dimenzija 85 bajta */

// /*  strukture za uslov */
// struct SUDopseg			 /* detektor u opsegu */
// {
//   //long det; MT 
//   uchar det4;
//   uchar det3;
//   uchar det2;
//   uchar det1;
//   uchar start;
//   uchar stop;
// };

// struct SUdogadjaj			 /* detektor u opsegu */
// {
//   uchar broj;
// };

// struct SUCRopseg		 /* ram lokacija u opsegu */
// {
//  // uint Cnst_leva;
//  // uint Cnst_desna;
// 	uchar Cnst_leva_h;
// 	uchar Cnst_leva_l;
	
// 	uchar Cnst_desna_h;
// 	uchar Cnst_desna_l;	
	
// 	uchar R;
// };

// struct SURRopseg		 /* ram lokacija u opsegu dve ram lokacije*/
// {
//   uchar R_levo;
//   uchar R;
//   uchar R_desno;
// };

// struct SURRporedj		 /* 'poredjenje' dve ram lokacije ( koje je poredjenje govori nam /tip.if_fun/ */
// {
//   uchar R_levo;
//   uchar R_desno;
// };

// struct SURi_ili		     /* ili f-ja svih RAM lokacija  */
// {
//   uchar R1;
//   uchar R2;
//   uchar R3;
//   uchar R4;
//   uchar R5;
//   uchar R6;
// };


// /*  strukture za obrada */
// struct SFRincrm			 /* inkrementiranje RAM lokacija */
// {
//   uchar R1;
//   uchar R2;
//   uchar R3;
//   uchar R4;
//   uchar R5;
//   uchar R6;
// };

// struct SFRincrm_cnt		 /* inkrementiranje RAM lokacija za konstantu */
// {
//  // sint     skok;
// 	schar skok_h;
// 	schar skok_l;
//   uchar     R;
// };

// struct SFRplus_ram		 /* inkrementiranje RAM lokacija za konstantu */
// {
//   uchar          Rleft;
//   uchar          R1;
//   uchar          R2;
//   uchar          R3;
//   uchar          R4;
//   uchar          R5;
// };

// struct SFRdel			 /* brisanje RAM lokacija */
// {
//   uchar R1;
//   uchar R2;
//   uchar R3;
//   uchar R4;
//   uchar R5;
//   uchar R6;
// };

// struct SFfaza_set		 /* pozivanje i setovanje naredne faze */
// {
//   uchar faza;
//   uchar tmin;
//   uchar topt;
//   uchar tmax;
//   uchar prelaz;
// };

// struct SFZTP_set		 /* pozivanje i setovanje naredne faze */
// {
//   uchar pgr_br;
// };

// struct SFdogadjaj		 /* dogadjaji */
// {
//   uchar broj;
// };

// #define BEZUSLOVNO	     1
// #define DETS_U_OPS	     2
// #define DOGADJ			 3
// #define RAM_U_CNST_OPS	 4
// #define RAM_U_RAM_OPS	 5
// #define RAM_VECE_RAM	 6
// #define RAM_VECJED_RAM	 7
// #define RAM_JEDNAKO_RAM	 8
// #define RAM_MANJJED_RAM	 9
// #define RAM_MANJE_RAM	10
// #define RAM_ILI	        11
// #define RAM_I	        12
// #define KOORD	        13


// union Udata_uslova
// {
//   uchar                data[ 6 ];
//   struct SUDopseg	   det_uCops;		  /* detektor u opsegu */
//   struct SUdogadjaj	   dogadjaj; 	      /* dogadjaj u opsegu */
//   struct SUCRopseg	   ram_uCops;	      /* ram lokacija u opsegu */
//   struct SURRopseg	   ram_uRops;	      /* ram lokacija u opsegu dve ram lokacije*/
//   struct SURRporedj	   RR_poredj;	      /* 'poredjenje' dve ram lokacije ( koje je poredjenje govori nam /tip.if_fun/ */
//   struct SURi_ili	      Ri_ili;	      /* ili f-ja svih RAM lokacija  */
// };


// #define DETS_INCR	     1
// #define SET_DOGADJ	     2
// #define RES_DOGADJ	     3
// #define RAM_INCR	     4
// #define RAM_CINCR	     5
// #define RAM_DEL	         6
// #define SET_FAZA	     7
// #define SET_ZTP	         8
// #define	RE_START	     9
// #define	RAM_RPLUSR	    10

// union Udata_iffun
// {
//   uchar                data[ 6 ];
//   struct SFdogadjaj   set_dogadj;	      /* setovanje dogadj */
//   struct SFdogadjaj   res_dogadj;	      /* setovanje dogadj */
//   struct SFRincrm	      increm;		  /* inkrementiranje RAM lokacija */
//   struct SFRincrm_cnt   inc_cnst;		  /* inkrementiranje RAM lokacija za konstantu */
//   struct SFRdel			   delet;         /* brisanje RAM lokacija */
//   struct SFfaza_set	    set_faze;         /* pozivanje i setovanje naredne faze */
//   struct SFZTP_set		 set_ztp;         /* pozivanje i setovanje naredne faze */
//   struct SFRplus_ram	  rplusr;	      /* inkrementiranje RAM lokacija za konstantu */
// };


// /********************************************************************************************************************/
// struct SA_sledeci 	                   
// {
	
//   uint  pozitivan : 5;	
//   uint  negativan : 5;	
//   uint  p_end     : 1;	
//   uint  n_end     : 1;	
//   uint  rezerva   : 4;	

  
// };

// union U_SA_sledeci
// {
// 	uchar tmp[2];
// 	struct SA_sledeci  sledeci;
// };

// /***********************************************************************************************************************/
// #define	CLR_LEFT	     1   // DET33_DET64		
// #define	CLR_RIGHT	     2   // OCC01_OCC32		//
// #define	CLR_LR	         3   // OCC33_OCC64		//
// #define	CLR_ALL_PROM	 4   // GAP01_GAP32		//
// #define	CLR_ALL_DET	     5   // GAP33_GAP64		//


// struct SA_cvor 
// {
//   uchar                  uslov;              
//   uchar             uslov_data;	
//   union Udata_uslova		 U;
//   uchar         	       fun;            
//   uchar               fun_data;	
//   union Udata_iffun	         F;
//   union U_SA_sledeci      u_SA;
//   //struct SA_sledeci    sledeci;	   /* bice realizovano preko pokazivaca */
// };


// #define   NULA	       0 * CNST + 1	      /* promenljive KSS-a */
// #define   BESKONACNO   0 * CNST + 2
// #define   SEKUNDA      0 * CNST + 3	  	  /* sekunda boravka u fazi / prel.sekvenci */
// #define	  Tcyc	   	   0 * CNST + 4		  /* sekunda kvazi ciklusa */
// #define   Tmin	   	   0 * CNST + 5
// #define	  Topt	   	   0 * CNST + 6
// #define   Tmax	   	   0 * CNST + 7
// #define	  Tps	   	   0 * CNST + 8
// #define	  DOGADJAJ	   0 * CNST + 9

// #define   START_FAZA   1 * CNST	+ 0			  /* parametri konkretne raskrsnice */
// #define   COORD_FAZA   1 * CNST	+ 1
// #define   DUZINA_CYC   1 * CNST	+ 2			  /* parametri konkretne raskrsnice */
// #define   START_SEC	   1 * CNST + 3

// /////////////   dopuna za "osigurane" detektore   ///////////

// #define   Derr1	       1 * CNST + 4          //    po bitima su poredjani "osigurani" detektori
// #define   Derr2	       1 * CNST + 5          //    po bitima su poredjani "osigurani" detektori

// #define   Dwar1	       1 * CNST + 6          //    po bitima su poredjani "warning" detektori
// #define   Dwar2	       1 * CNST + 7          //    po bitima su poredjani "warning" detektori

// /////////////////////////////////////////////////////////////
								  
// #define	  DET1	   	   3 * CNST + 0	 /* promenljive vezane za detektore */ 
// #define	  DET2	   	   3 * CNST + 1
// #define	  DET3	   	   3 * CNST + 2
// #define	  DET4	   	   3 * CNST + 3
// #define	  DET5	   	   3 * CNST + 4
// #define	  DET6	   	   3 * CNST + 5
// #define	  DET7	   	   3 * CNST + 6
// #define	  DET8	   	   3 * CNST + 7
// #define	  DET9	   	   3 * CNST + 8
// #define	  DET10	   	   3 * CNST + 9
// #define	  DET11	   	   3 * CNST + 10
// #define	  DET12	   	   3 * CNST + 11
// #define	  DET13	   	   3 * CNST + 12
// #define	  DET14	   	   3 * CNST + 13
// #define	  DET15	   	   3 * CNST + 14
// #define	  DET16	   	   3 * CNST + 15

// #define	  DET17	   	   4 * CNST + 0	  
// #define	  DET18	   	   4 * CNST + 1
// #define	  DET19	   	   4 * CNST + 2
// #define	  DET20	   	   4 * CNST + 3
// #define	  DET21	   	   4 * CNST + 4
// #define	  DET22	   	   4 * CNST + 5
// #define	  DET23	   	   4 * CNST + 6
// #define	  DET24	   	   4 * CNST + 7
// #define	  DET25	   	   4 * CNST + 8
// #define	  DET26	   	   4 * CNST + 9
// #define	  DET27	   	   4 * CNST + 10
// #define	  DET28	   	   4 * CNST + 11
// #define	  DET29	   	   4 * CNST + 12
// #define	  DET30	   	   4 * CNST + 13
// #define	  DET31	   	   4 * CNST + 14
// #define	  DET32	   	   4 * CNST + 15  


// #define	  OCC1	   	   5 * CNST + 0	        /* promenljive vezane za zauzece */ 

// /*  #define	  ZZ2	   	   5 * CNST + 1	  
//     -------------
//     #define	  ZZ32	   	   6 * CNST + 15  */


// #define	  GAP1	   	   7 * CNST + 0	         /* promenljive vezane za zauzece */ 
// #define	  GAP32	   	   8 * CNST + 15
// /*  #define	  GAP2	   	   7 * CNST + 1	  
//     -------------
//     #define	  GAP32	   	   8 * CNST + 15  */


// #define	  D1	 0x0001
// #define	  D2	 0x0002
// #define	  D3	 0x0004
// #define	  D4	 0x0008
// #define	  D5	 0x0010
// #define	  D6	 0x0020
// #define	  D7	 0x0040
// #define	  D8	 0x0080

// /*#define	  D9	 (long) 0x00000100
// #define	  D10	 (long) 0x00000200
// #define	  D11	 (long) 0x00000400
// #define	  D12	 (long) 0x00000800
// #define	  D13	 (long) 0x00001000
// #define	  D14	 (long) 0x00002000
// #define	  D15	 (long) 0x00004000
// #define	  D16	 (long) 0x00008000
// #define	  D17	 (long) 0x00010000
// #define	  D18	 (long) 0x00020000
// #define	  D19	 (long) 0x00040000
// #define	  D20	 (long) 0x00080000
// #define	  D21	 (long) 0x00100000
// #define	  D22	 (long) 0x00200000
// #define	  D23	 (long) 0x00400000
// #define	  D24	 (long) 0x00800000
// #define	  D25	 (long) 0x01000000
// #define	  D26	 (long) 0x02000000
// #define	  D27	 (long) 0x04000000
// #define	  D28	 (long) 0x08000000
// #define	  D29	 (long) 0x10000000
// #define	  D30	 (long) 0x20000000
// #define	  D31	 (long) 0x40000000
// #define	  D32	 (long) 0x80000000
// */


// #define BR_USLOVA		  16
// #define BR_OBRADA		  16




// #endif /* DETEKTOR_H_ */
