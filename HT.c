/**************************************************************************
 * Ohjelma: Itseään tasapainoittava binäärinen hakupuu
 * Tekijä: Hugo Hutri
 * Aika: Marraskuu 2019
 * Lähteet:
 *      M. Penttonen, Johdatus algoritmien suunnitteluun ja analysointiin
 *      A. Laaksonen, https://www.cs.helsinki.fi/u/ahslaaks/tirakirja/
 **************************************************************************/

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

typedef struct SOLMU {
    int avain;
    int korkeus;
    struct SOLMU *vasen;
    struct SOLMU *oikea;
} Solmu;

void vapautaPuunMuisti(Solmu*);
void tulostaPuu(Solmu*);
void tulostaRivi(Solmu*, int, int);
void tulostaViivat(Solmu*, int, int, int, int);
int lisaaSolmu(Solmu**, int);
void oikeaKierto(Solmu**);
void vasenKierto(Solmu**);
int laskeTasapaino(Solmu*);
int laskeKorkeus(Solmu*);
int solmunKorkeus(Solmu*);
int maxi(int, int);
int pow2(int);
Solmu* etsiAvain(Solmu*, int);
void tulostaValikko();


int main(int argc, char **argv) {
    Solmu* juuri = NULL;
    FILE* tiedosto;
    int avain, valinta = 0;

    if(argc < 2) {
        printf("Argumentti puuttuu\n");
        return 0;
    }

    /* Luetaan luvut tiedostosta puuhun */
    if(!(tiedosto = fopen(argv[1], "r"))) {
        perror("Virhe");
        return 0;
    }

    while (fscanf(tiedosto, "%d\n", &avain) != EOF){
        lisaaSolmu(&juuri, avain);
        tulostaPuu(juuri);
    }
    fclose(tiedosto);

    /* Valikko */
    do {
        tulostaValikko();
        scanf("%d",&valinta);
        switch (valinta) {
        case 1:
            printf("Lisattava avain: ");
            scanf("%d",&avain);
            lisaaSolmu(&juuri, avain);
            tulostaPuu(juuri);
            break;
        case 2:
            printf("Haettava avain: ");
            scanf("%d",&avain);
            Solmu* solmu = etsiAvain(juuri, avain);
            if(solmu != NULL) printf("Avain loytyi: %d\n", solmu->avain);
            else printf("Avainta ei loytynyt\n");
            break;
        case 3:
            tulostaPuu(juuri);
            break;
        case 0:
            printf("Lopetetaan\n");
            break;
        default:
            printf("Virheellinen valinta\n");
            break;
        }
    } while(valinta != 0);


    /* Lopetus ja muistin vapauttaminen */
    printf("Vapautetaan solmut: ");
    vapautaPuunMuisti(juuri);
    printf("\n");
    return 0;
}

/***********************************************************
 * Etsitään avain rekursiivisesti. Kompleksisuus on O(log n)
 * Palauttaa: Löydetyn solmun
 ***********************************************************/
Solmu* etsiAvain(Solmu *solmu, int avain) {
    if(solmu == NULL)           return NULL;
    if(solmu->avain == avain)   return solmu;
    if(avain < solmu->avain)    return etsiAvain(solmu->vasen, avain);
    if(avain > solmu->avain)    return etsiAvain(solmu->oikea, avain);
    return NULL;
}

/*****************************
 * Lisätään puuhun solmu
 * Palauttaa: Solmun korkeuden
 *****************************/
int lisaaSolmu(Solmu **solmu, int avain) {
    /* Luodaan uusi solmu */
    if((*solmu) == NULL) {
        if(((*solmu) = (Solmu*) malloc(sizeof(Solmu))) == NULL) {
            perror("Muistin varaaminen ei onnistunut \n");
            exit(1);
        }
        printf("Lisataan solmu %d\n",avain);
        /* Asetetaan arvot */
        (*solmu)->avain = avain;
        (*solmu)->korkeus = 0;
        (*solmu)->vasen = NULL;
        (*solmu)->oikea = NULL;
        /* Palautetaan korkeus */
        return 0;
    }


    /* Lisättävä avain on suurempi kuin solmun avain */
    /* Lisätään solmu oikealle */
    if(avain > (*solmu)->avain) {
        int korkeus = lisaaSolmu(&((*solmu)->oikea), avain);
        (*solmu)->korkeus = maxi(korkeus+1 , (*solmu)->korkeus);
    } 

    /* Lisättävä solmu on pienempi kuin solmun avain */
    /* Lisätään solmu vasemmalle */
    else if(avain < (*solmu)->avain) {
        int korkeus = lisaaSolmu(&((*solmu)->vasen), avain);
        (*solmu)->korkeus = maxi(korkeus+1 , (*solmu)->korkeus);
    }

    /* Solmun avain on sama kuin syötettävä avain */
    /* Ei lisätä uutta solmua */
    else {
        printf("Solmu on jo olemassa\n");
    }

    /* Tarkistetaan tasapaino */
    /* ja tehdään kierrot */
    int ero = laskeTasapaino(*solmu);

    if(ero > 1) {
        printf("Solmun %d painotus oikealla!\n", (*solmu)->avain);
        oikeaKierto(solmu);
    }
    else if (ero < -1) {
        printf("Solmun %d painotus vasemmalla!\n", (*solmu)->avain);
        vasenKierto(solmu);
    }

    return (*solmu)->korkeus;
}

/***********************************
 * Puun tasapainoitus vasenkierrolla
 ***********************************/
void vasenKierto(Solmu **ylin) {
    Solmu *keski = (*ylin)->vasen;
    int ero = laskeTasapaino(keski);
    if(ero < 0) {
        /********************
         * Vasen kierto
         *            ylin
         *           /   \
         *       keski
         *      /    \
         * 
         *********************/
        printf("Suoritetaan vasenkierto \n");
        (*ylin)->vasen = keski->oikea;
        keski->oikea = (*ylin);
        (*ylin)->korkeus = laskeKorkeus(*ylin);
        /* Päivitetään keski-solmu uudeksi ylin-solmuksi ja lasketaan sille uusi korkeus */
        *ylin = keski;
        (*ylin)->korkeus = laskeKorkeus(*ylin);
    } else {
        /********************
         * Vasen-oikea kierto
         *            ylin
         *           /   \
         *       keski
         *      /    \
         *            alin
         *********************/
        printf("Suoritetaan vasen-oikeakierto \n");
        Solmu* alin = keski->oikea;
        
        /* Oikeakierto keski-solmulle */
        keski->oikea = alin->vasen;
        alin->vasen = keski;
        keski->korkeus = laskeKorkeus(keski);
        keski = alin;
        keski->korkeus = laskeKorkeus(keski);

        /* Vasenkierto ylin-solmulle */
        (*ylin)->vasen = keski->oikea;
        keski->oikea = (*ylin);
        (*ylin)->korkeus = laskeKorkeus(*ylin);
        *ylin = keski;
        (*ylin)->korkeus = laskeKorkeus(*ylin);
    }
}

/***********************************
 * Puun tasapainoitus oikeakierrolla
 ***********************************/
void oikeaKierto(Solmu **ylin) {
    Solmu *keski = (*ylin)->oikea;
    int ero = laskeTasapaino(keski);
    if(ero > 0) {
        /********************
         * oikea kierto
         *   ylin
         *   /   \
         *       keski
         *      /    \
         * 
         *********************/
        printf("Suoritetaan oikeakierto \n");
        (*ylin)->oikea = keski->vasen;
        keski->vasen = (*ylin);
        (*ylin)->korkeus = laskeKorkeus(*ylin);
        *ylin = keski;
        (*ylin)->korkeus = laskeKorkeus(*ylin);
    } else {
        /********************
         * oikea-vasen kierto
         *   ylin
         *  /   \
         *       keski
         *      /    \
         *   alin
         *********************/
        printf("Suoritetaan oikea-vasenkierto \n");
        Solmu* alin = keski->vasen;
        
        /* vasenkierto keski-solmulle */
        keski->vasen = alin->oikea; 
        alin->oikea = keski;
        keski->korkeus = laskeKorkeus(keski);
        keski = alin;
        keski->korkeus = laskeKorkeus(keski);

        /* oikeakierto ylin-solmulle */
        (*ylin)->oikea = keski->vasen;
        keski->vasen = (*ylin);
        (*ylin)->korkeus = laskeKorkeus(*ylin);
        *ylin = keski;
        (*ylin)->korkeus = laskeKorkeus(*ylin);
    }
}

/*************************************
 * Laskee korkeuden lapsisolmuista
 * Palauttaa: korkeuden
 *************************************/
int laskeKorkeus(Solmu* solmu) {
    /* Jos solmu on NULL, sen korkeus on -1, eli vanhemman korkeus on 0*/
    if(solmu == NULL) return -1;
    int korkeus_oik = solmunKorkeus(solmu->oikea);
    int korkeus_vas = solmunKorkeus(solmu->vasen);
    int korkeus = maxi(korkeus_oik,korkeus_vas) + 1;
    return korkeus;
}

/**********************************
 * Vapautetaan puun varaama muisti
 **********************************/
void vapautaPuunMuisti(Solmu *solmu) {
	if(solmu == NULL) return;

	vapautaPuunMuisti(solmu->vasen);
    vapautaPuunMuisti(solmu->oikea);

    printf("%d ",solmu->avain);
    free(solmu);
}

/**********************************
 * Apufunktio selkeyttämään laskuja
 * Palauttaa: Solmun korkeuden
 **********************************/
int solmunKorkeus(Solmu* solmu) {
    if(solmu == NULL) return -1;
    return solmu->korkeus;
}

/**********************************
 * Tulostetaan binääripuun rivi
 **********************************/
void tulostaRivi(Solmu *solmu, int korkeus, int tavoite) {
    if(solmu == NULL) {
        int k;
        for(k = 0; k < (pow2(korkeus-tavoite)); k++) {
            printf("  ");
            int i;
            for(i = 0; i < (pow2(tavoite+1)-1)*2; i++) printf(" ");
        }
        return;
    }
    else if(korkeus == tavoite) {
        printf("%02d",solmu->avain);
        int i;
        for(i = 0; i < (pow2(korkeus+1)-1)*2; i++) printf(" ");
        return;
    }
    korkeus--;
    tulostaRivi(solmu->vasen, korkeus, tavoite);
    tulostaRivi(solmu->oikea, korkeus, tavoite);
}

/**********************************
 * Tulostetaan binääripuun viivat
 **********************************/
void tulostaViivat(Solmu *solmu, int korkeus, int tavoite, int suunta, int valit) {
    int i;
    if(solmu == NULL) {
        int k;
        for(k = 0; k < (pow2(korkeus-tavoite)); k++) {
            if(suunta < 0) printf(" ");
            else if(suunta > 0) {
                printf(" ");
                int paikkaus = pow2(tavoite+3)-2-2*valit;
                for(i = 0; i < paikkaus; i++) printf(" ");
            }
            for(i = 0; i < valit; i++) printf(" ");
            suunta *= -1;
        }
        return;
    }
    else if(korkeus == tavoite) {
        if(suunta < 0) printf("/");
        else if(suunta > 0) {
            printf("\\");
            int paikkaus = pow2(korkeus+3)-2-2*valit;
            for(i = 0; i < paikkaus; i++) printf(" ");

        }
        for(i = 0; i < valit; i++) printf(" ");
        return;
    }
    korkeus--;
    tulostaViivat(solmu->vasen, korkeus, tavoite, -1, valit);
    tulostaViivat(solmu->oikea, korkeus, tavoite, 1, valit);
}

/**********************************
 * Tulostetaan koko binääripuu
 **********************************/
void tulostaPuu(Solmu* juuri) {
    int i, k, kerros, korkeus;
    korkeus = juuri->korkeus;

    for(kerros = korkeus; kerros >= 0; kerros--) {
        /* Tulostetaan binääripuu kerroksittain */
        int maxi = pow2(kerros+1)-1;
        for(k = 0; k < maxi; k++) {
            if(korkeus == kerros) break;
            printf("\n    ");
            for(i = 1; i < 2*maxi-k; i++) printf(" ");
            int valit = (k+1)*2;
            tulostaViivat(juuri, korkeus, kerros, 0, valit);
        }
        printf("\n    ");
        for(i = 0; i < (pow2(kerros)-1)*2; i++) printf(" ");
        tulostaRivi(juuri, korkeus, kerros);

    }
    printf("\n\n--------------------------------------------------------------------------\n");
}

/************************************************
 * Lasketaan solmun tasapaino lasten korkeudesta
 * Palauttaa: Tasapainon erotuksena
 ************************************************/
int laskeTasapaino(Solmu *solmu) {
    int korkeus_oik = solmunKorkeus(solmu->oikea);
    int korkeus_vas = solmunKorkeus(solmu->vasen);
    int erotus = korkeus_oik - korkeus_vas;
    return erotus;
}

/*********************
 * Kahden potenssi
 * Palauttaa: 2^n
 *********************/
int pow2(int luku) {
    int tulos = 1;
    int i;
    for(i = 0; i < luku; i++)
        tulos *= 2;
    return tulos;
}

/**************************
 * Maxi-funktio integereille
 * Palauttaa: Suuremman luvun
 **************************/
int maxi(int a, int b) {
    return (a > b) ? a : b;
}

/*************************
 * Valikon tulostaminen
 *************************/
void tulostaValikko() {
    printf("\nValikko:\n");
    printf("1) Lisaa avain puuhun\n");
    printf("2) Hae avainta puusta\n");
    printf("3) Tulosta puu\n");
    printf("0) Lopeta\n");
    printf("Valintasi: ");
}
