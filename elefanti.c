#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <pthread.h>

#include "DBGpthread.h"
#include "printerror.h"

#define CAPIENZA_POZZA 500
#define ELEFANTI 5
#define L_BEVUTI_DA_ELEFANTI 100
#define PEKARI 10
#define L_BEVUTI_DA_PEKARI 1
#define L_AGGIUNTI_DA_ACQUEDOTTO 70

int nElefantiInBevuta = 0;
int nPekariInBevuta = 0;
int litriPozza = CAPIENZA_POZZA;

pthread_mutex_t mutex;
pthread_cond_t condElefanti;
pthread_cond_t condPekari;


int calcoloAcquaNecessaria( int pekari, int elefanti ) {
    return ( pekari * L_BEVUTI_DA_PEKARI ) + ( elefanti * L_BEVUTI_DA_ELEFANTI );
}

void *pekari( void *arg ) {

    char Plabel[128];
    sprintf( Plabel, "Pekari %" PRIiPTR "", ( intptr_t )arg );

    while( 1 ) {
        DBGpthread_mutex_lock( &mutex, Plabel );
        printf( "%s: vorrei bere\n", Plabel );
        while( litriPozza < calcoloAcquaNecessaria( nPekariInBevuta+1, nElefantiInBevuta ) ){
            printf( "%s: Pozza non capiente abbastanza per fare bere anche me (litri rimasti %d, necessari %d)\n", Plabel, litriPozza, calcoloAcquaNecessaria( nPekariInBevuta+1, nElefantiInBevuta ) );
            DBGpthread_cond_wait( &condPekari, &mutex, Plabel );
        }
        nPekariInBevuta++;
        printf( "%s: Posso bere (elfeanti in bevuta %d, pekari in bevuta %d)\n", Plabel, nElefantiInBevuta, nPekariInBevuta );
        DBGpthread_mutex_unlock( &mutex, Plabel );
        DBGnanosleep( 10000000, Plabel );
        DBGpthread_mutex_lock( &mutex, Plabel );
        litriPozza -= L_BEVUTI_DA_PEKARI;
        nPekariInBevuta--;
        printf( "%s: ho finito di bere\n", Plabel );
        if( nPekariInBevuta == 0 ) {
            printf( "%s: ero l'ultimo pekaro in bevuta, sveglio gli elefanti\n", Plabel );
            DBGpthread_cond_broadcast( &condElefanti, Plabel );
        }
        DBGpthread_mutex_unlock( &mutex, Plabel );
       DBGnanosleep( 2200000000, Plabel );
    }

    pthread_exit( NULL );
}

void *elefante( void *arg ) {

    char Elabel[128];
    sprintf( Elabel, "Elefante %" PRIiPTR "", ( intptr_t )arg );

    while( 1 ) {
        DBGpthread_mutex_lock( &mutex, Elabel );
        printf( "%s: vorrei bere\n", Elabel );
        while( nPekariInBevuta > 0 || litriPozza < calcoloAcquaNecessaria( nPekariInBevuta, nElefantiInBevuta+1 ) ) {
            printf( "%s: Pozza non capiente abbastanza per fare bere anche me (litri rimasti %d, necessari %d) o pekari in bevuta ( %d )\n", Elabel, litriPozza, calcoloAcquaNecessaria( nPekariInBevuta, nElefantiInBevuta+1 ), nPekariInBevuta );
            DBGpthread_cond_wait( &condElefanti, &mutex, Elabel );
        }
        nElefantiInBevuta++;
        printf( "%s: Posso bere (elfeanti in bevuta %d, pekari in bevuta %d)\n", Elabel, nElefantiInBevuta, nPekariInBevuta );
        DBGpthread_mutex_unlock( &mutex, Elabel );
        DBGnanosleep( 100000000, Elabel );
        DBGpthread_mutex_lock( &mutex, Elabel );
        litriPozza -= L_BEVUTI_DA_ELEFANTI;
        nElefantiInBevuta--;
        printf( "%s: ho finito di bere, sveglio i pekari... magari c'e' ancora acqua\n", Elabel );
        DBGpthread_cond_broadcast( &condPekari, Elabel );
        DBGpthread_mutex_unlock( &mutex, Elabel );
        DBGnanosleep( 1000000000, Elabel );
    }

    pthread_exit( NULL );
}

void *acquedotto( void *arg ) {

    char Alabel[128];
    sprintf( Alabel, "Acquedotto" );

    while( 1 ) {
        DBGpthread_mutex_lock( &mutex, Alabel );
        printf( "%s: Pozza rifornita!\n", Alabel );
        litriPozza += (litriPozza + L_AGGIUNTI_DA_ACQUEDOTTO > CAPIENZA_POZZA ? CAPIENZA_POZZA : L_AGGIUNTI_DA_ACQUEDOTTO);
        DBGpthread_cond_broadcast( &condPekari, Alabel );
        DBGpthread_cond_broadcast( &condElefanti, Alabel );
        DBGpthread_mutex_unlock( &mutex, Alabel );
        DBGnanosleep( 1000000000, Alabel );
    }

    pthread_exit( NULL );
}

int main( void ) {
    int rc;
    intptr_t i;
    pthread_t th;

    DBGpthread_mutex_init( &mutex, NULL, "main" );
    DBGpthread_cond_init( &condElefanti, NULL, "main" );
    DBGpthread_cond_init( &condPekari, NULL, "main" );

    /* Creazione pekari */
    for( i=0; i<PEKARI; i++ ) {
        rc = pthread_create( &th, NULL, pekari, ( void *)i );
        if( rc ) {
            PrintERROR_andExit( rc, "Creazione pekari" );
        }
    }

    /* Creazione elefanti */
    for( i=0; i<ELEFANTI; i++ ) {
        rc = pthread_create( &th, NULL, elefante, ( void *)i );
        if( rc ) {
            PrintERROR_andExit( rc, "Creazione elefante" );
        }
    }

    /* creazione acquedotto */
    rc = pthread_create( &th, NULL, acquedotto, NULL );
        if( rc ) {
            PrintERROR_andExit( rc, "Creazione acquedotto" );
        }

    pthread_exit( NULL );
    return 0;

}
