#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE    3000
#define MAX(x,y) ((x)>(y)?(x):(y))

#define BASE_ALT -100

/* make this a global to avoid stack overflow */
int altitude[MAX_SIZE][MAX_SIZE];

/* This function writes out the crossfire maps.  So shoot me for
 * using compiled in constants - I'm not going to use this so much
 * that I wanted to do anything too easy.
 */

#define MAP_FORMAT  "world_%03d_%03d"

/* Maps are square */

#define MAP_SIZE    50

/* There will be a total of 2500 maps (eek) - 50 in
 * each diretion.  STARTX and STARTY are where to start
 * numbering from.  I chose 100 for a few reasons - 1) it
 * gives room to the left and above to add some things (another
 * continent for that matter), and 2) since the format allows
 * for up to 1000 in each direction, this seemed reasonable.
 * Note - if you do the math, and have 1000 * 1000 maps, each
 * with 50*50 spaces, you have a total of 2.5 billion spaces.
 * So hopefully that should be large enough.
 */

#define STARTX	100
#define STARTY	100

typedef enum {
    None=0,
    DeepWater=1,
    MediumWater=2,
    ShallowWater=3,
    Swamp=4,
    DeepSwamp=5,
    Grass=6,
    Desert=7,
    Brush=8,
    EverGreens=9,
    Jungle=10,
    Tree1=11,
    Tree2=12,
    Woods1=13,
    Woods2=14,
    Woods3=15,
    Hills=16,
    HillsRocky=17,
    Steppe=17,
    Mountain=19,
    HighMountain=20,
    WasteLand=21
} Terrain_Types;

char *Terrain_Names[][2] = {
    /* these are the archetype names.  They are in the same order
     * as the Terrain_Types above.  Note many terrain types are not
     * included because handling them would be too difficult.
     */
    {"None",	    "0, 0, 0 "},
    {"deep_sea",    "0 0 127 "},
    {"sea",	    "0 0 192 "},
    {"shallow_sea", "0 0 255 "}, /* wading depth */
    {"swamp",	    "12 161 64 "},
    {"deep_swamp",  "155 175 164 "},
    {"grass",	    "0 255 0 "},
    {"desert",	    "222 218 135 "},
    {"brush",	    "1 144 1 "},
    {"evergreens",  "0 128 0 "},
    {"jungle_1",    "0 176 0 "},
    {"tree",	    "4 133 01 "},
    {"evergreen",   "20 209 0 "},
    {"woods",	    "4 115 01 "},
    {"woods_2",	    "1 182 02 "},
    {"woods_3",	    "4 153 02 "},
    {"hills",	    "166 160 70 "},
    {"hills_rocky", "166 155 70 "},
    {"steppe",	    "150 97 34 "},
    {"mountain",    "183 190 190 "},
    {"mountain2",   "191 196 185 "},
    {"wasteland",   "255 255 255 "},
};
 


void write_crossfire_maps(int mapx, int mapy)
{
    int x, y,n,q, nx, ny,r1,r2,ax=0,ay=0, j, k;
    char name[512];
    FILE    *fp;
    Terrain_Types *terrain;

    terrain = calloc(mapx * mapy * sizeof(Terrain_Types), sizeof(Terrain_Types));

    /* First fill in the water and the highest of peaks */
    for (x=0; x<mapx; x++) {
	for (y=0; y<mapy; y++) {
	    if (altitude[y][x] < -5000) {
		terrain[x + y * mapx] = DeepWater;
	    } else if (altitude[y][x] < -99) {
		terrain[x + y * mapx] = MediumWater;
	    } else if (altitude[y][x] < 1) {
		terrain[x + y * mapx] = ShallowWater;
	    } else if (altitude[y][x] >=12000) {
		/* Not really precisely wasteland, but wastelands are impassable */
		terrain[x + y * mapx] = WasteLand;
	    }
	}
    }
    /* Basically, take a random bit and populate the area with terrain.
     * We do this so it won't be totally monolythic (have several forest types
     * for example), yet patches will be the same thing, eg, a stretch of
     * desert, which wouldn't work very well if we just chose randomly
     * for each space.
     */

    for (n=0; n<(mapx * mapy) / 100; n++) {
	do {
	    x = random() % mapx;
	    y = random() % mapy;
	} while ( terrain[x + y * mapx] == None);

	nx = x + 40;
	if (nx > mapx) nx=mapx;
	ny = y + 40;
	if (ny > mapy) ny = mapy;
	r1 = random();
	r2 = random();
	for (x = nx-40; x<nx; x++) {
	    for (y=ny-40; y<ny; y++) {
		if (terrain[x + y * mapx] != None) continue;

		/* near the edges, don't always fill in so that hopefully something
		 * else will fill in and smooth these out some
		 */
		if  ( (x < (nx -30) || y < (ny - 30) || x > (nx -10) || y > (ny - 10)) &&
		     random() % 2) continue;

		if (altitude[y][x] < 10) {
		    terrain[x + y * mapx] = Swamp + (r1 % 2);
		}
		else if (altitude[y][x] < 1000) {
		    terrain[x + y * mapx] = Grass + (r1 % 3);
		} else if (altitude[y][x] < 3000) {
		    terrain[x + y * mapx] = EverGreens + (r1 % 9);
		} else if (altitude[y][x] < 5000) {
		    terrain[x + y * mapx] = Hills + (r2 % 3);
		} else if (altitude[y][x] < 9000) {
		    terrain[x + y * mapx] = Mountain;
		} else if (altitude[y][x] < 12000) {
		    terrain[x + y * mapx] = HighMountain;
		}
		else fprintf(stderr,"altitude %d did not get filled in?\n", altitude[y][x]);
	    }
	}
    }
    /* Now just fill in the spaces randomly. */
    n=0;
    r1 = random();
    r2 = random();
    for (x=0; x<mapx; x++) {
	for (y=0; y<mapy; y++) {
	    if (terrain[x + y * mapx] != None) continue;
	    n++;
	    if (altitude[y][x] < 10) {
		terrain[x + y * mapx] = Swamp + (r1 % 2);
	    }
	    if (altitude[y][x] < 1000) {
		terrain[x + y * mapx] = Grass + (r1 % 3);
	    } else if (altitude[y][x] < 3000) {
		    terrain[x + y * mapx] = EverGreens + (r2 % 9);
	    } else if (altitude[y][x] < 5000) {
		    terrain[x + y * mapx] = Hills + (r2 % 3);
	    } else if (altitude[y][x] < 9000) {
		    terrain[x + y * mapx] = Mountain;
	    } else if (altitude[y][x] < 12000) {
		    terrain[x + y * mapx] = HighMountain;
	    }
	}
    }
    fprintf(stderr,"Filled in %d spaces\n",n);
    if ((mapx / MAP_SIZE) * MAP_SIZE != mapx ||
	(mapy / MAP_SIZE) * MAP_SIZE != mapy) {
	fprintf(stderr,"Warning - generated map does not evenly tile.\n");
    }
    for (nx= STARTX; nx<(STARTX + (mapx/ MAP_SIZE)); nx++) {
	for (ny= STARTY; ny<(STARTY + (mapy/ MAP_SIZE)); ny++) {
	    ax = (nx - STARTX) * MAP_SIZE;
	    ay = (ny - STARTY) * MAP_SIZE;

	    sprintf(name,MAP_FORMAT,nx,ny);
	    if ((fp=fopen(name, "w"))==NULL) {
		fprintf(stderr,"unable to open %s\n", name);
	    }
	    /* Write the header for the map */
	    fprintf(fp,"arch map\n");
	    fprintf(fp,"name %s\n", name);
	    fprintf(fp,"width %d\n", MAP_SIZE);
	    fprintf(fp,"height %d\n", MAP_SIZE);
	    /* Not used right now, but useful to include */
	    fprintf(fp,"outdoor 1\n", MAP_SIZE);

	    /* don't do difficult, reset time, or enter coordinates */
	    /* Set up the tile paths */
	    if (ny != STARTY) {
		fprintf(fp,"tile_path_1 ");
		fprintf(fp,MAP_FORMAT,nx, ny-1);
		fprintf(fp,"\n");
	    }
	    if ((nx+1) < STARTX + (mapx/ MAP_SIZE)) {
		fprintf(fp,"tile_path_2 ");
		fprintf(fp,MAP_FORMAT,nx+1, ny);
		fprintf(fp,"\n");
	    }
	    if ((ny+1) < STARTY + (mapy/ MAP_SIZE)) {
		fprintf(fp,"tile_path_3 ");
		fprintf(fp,MAP_FORMAT,nx, ny+1);
		fprintf(fp,"\n");
	    }
	    if (nx != STARTX) {
		fprintf(fp,"tile_path_4 ");
		fprintf(fp,MAP_FORMAT,nx-1, ny);
		fprintf(fp,"\n");
	    }
	    fprintf(fp,"end\n");
	    for (x = 0; x<50; x++) {
		for (y = 0; y < 50; y++) {
		    q = terrain[x + ax  + (y + ay)* mapx];
		    fprintf(fp, "arch %s\n",Terrain_Names[q][0]);
		    fprintf(fp,"x %d\n", x);
		    fprintf(fp,"y %d\n", y);
		    q = altitude[y + ay ][x + ax];
		    if (q< -32000) q = -32000;
		    if (q > 32000) q = 32000;
		    fprintf(fp,"elevation %d\n", q);
		    fprintf(fp,"end\n");
		}
	    }
	    fclose(fp);
	}
    }

    fp = fopen("cmap", "w");
    fprintf(fp, "P3 %d %d 255\n", mapy, mapx);
    for (y=0; y < mapy; y++) {
	    for (x=0; x < mapx; x++) {
		fprintf(fp, Terrain_Names[terrain[x + y * mapx]][1]);
	    }
	fprintf(fp, "\n");
	}
    exit(0);
}



main(int argc, char *argv)
{
    int x, y, max_x=500, max_y=500, seed, land=300000, npasses=40, newalt, wpasses=50, water=50000;
    int n, i, j, k, l, z, w, r, a, write_maps=0;
    FILE *fp, *lp;
    int junk;
    char c;
    extern char *optarg;

    seed = time(NULL);
    while ((c = getopt(argc, argv,"x:y:s:l:n:w:p:m"))!=-1) {
	switch (c) {
	    case 'l':
		land = atoi(optarg);
		if (land < 11 ) {
		    fprintf(stderr,"-l must be at least 11\n");
		    exit(1);
		}
		break;

	    case 'w':
		water = atoi(optarg);
		if (water < 1 ) {
		    fprintf(stderr,"-w must be at least 1\n");
		    exit(1);
		}
		break;

	    case 'p':
		wpasses = atoi(optarg);
		if (wpasses < 1 ) {
		    fprintf(stderr,"-w must be at least 1\n");
		    exit(1);
		}
		break;

	    case 'n':
		npasses = atoi(optarg);
		if (npasses < 10 ) {
		    fprintf(stderr,"-n must be at least 10\n");
		    exit(1);
		}
		break;

	    case 'x':
		max_x = atoi(optarg);
		break;

	    case 'y':
		max_y = atoi(optarg);
		break;

	    case 's':
		seed = atoi(optarg);
		break;

	    case 'm':
		write_maps=1;
		break;
	}
    }
    if (max_x > MAX_SIZE || max_y > MAX_SIZE) {
	fprintf(stderr,"Max X and Y size is %d\n", MAX_SIZE);
	exit(1);
    }

    fprintf(stderr,"Making %d X %d map, seed %d, land %d, passes = %d\n", max_x, max_y, seed, land, npasses);
    fprintf(stderr,"wpasses =%d, water=%d\n", wpasses, water);
    fprintf(stderr,"-x %d -y %d -s %d -p %d -n %d -w %d -l %d\n",
	    max_x, max_y, seed, wpasses, npasses, water, land);

    srandom(seed);

    for (x=20; x < max_x-20;  x++)
	for (y=20; y < max_y-20; y++)
	    altitude[x][y] = BASE_ALT;

    for (x=0; x<max_x; x++) {
	for (y=0; y<20; y++) {
	    altitude[x][y] = (y -20 ) * 100;
	    altitude[x][max_y - y] =  (y -20 ) * 100;
	}
    }

    for (y=10; y<max_y-10; y++) {
	for (x=0; x<20; x++) {
	    altitude[x][y] = (x - 20) * 100;
	    altitude[max_x - x][y] =  (x - 20) * 100;
	}
    }
  
    /* This basically produces areas of high varience (eg, islands, peaks, valleys, etc) */

    for (l=0; l<npasses; l++) {
	x = random()%max_x;
	y = random()%max_y;
	/* Weigh our selected starting position a little more towards the center
	 * so the continent is more in the center
	 */
	if (random() % 2) {
	    x += random()%max_x;
	    y += random()%max_y;
	    x /=2;
	    y /=2;
	}
	n = random()%500+800;

	/* For some portion, try to find a pixel we have yet to modify */
	if (l> (npasses * 15) / 20) {
	    int tries=0;
	    while (altitude[x][y] == BASE_ALT) {
		x = random()%max_x;
		y = random()%max_y;
		if (random() % 2) {
		    x += random()%max_x;
		    y += random()%max_y;
		    x /=2;
		    y /=2;
		}
		tries++;
		if (tries > 50) {
		    fprintf(stderr,"did not find free space within %d tries\n", tries);
		    break;
		}
	    }

	}

	for (k=1; k< land ; k++) {
	    r = random()%4;
	    switch (r) {
		case 0: if (x<max_x-1) x++; else x -= random() % (max_x/2); break;
		case 1: if (y<max_y-1) y++; else y -= random() % (max_y/2);  break;
		case 2: if (x) x--; else x+= random() % (max_x/2); break;
		case 3: if (y) y--; else y+= random() % (max_y/2); break;
	    }
	    altitude[x][y] += n;
	    if (random()%k < 100)
		n -= 1;
	}
    }

    /* Make lakes and ocean trenches.
     * General note - it works better to have more passes, but each
     * pass doing less working - this results in more consistent lakes
     * and ocen trenching.
     */
    for (l=0; l<wpasses; l++) {
	/* for a small portion, we lower the area we make */
	    n = random()%1500-2000;

	    x = random()% max_x;
	    y = random()% max_y;

	    while (altitude[x][y] > BASE_ALT || altitude[x][y]<-7000) {
		x = random()% max_x;
		y = random()% max_y;
	    }
	    for (k=1; k< water ; k++) {
		r = random()%4;
		switch (r) {
		    case 0: if (x<max_x-1) x++;  break;
		    case 1: if (y<max_y-1) y++;   break;
		    case 2: if (x) x--;  break;
		    case 3: if (y) y--;  break;
		}
		altitude[x][y] += n;
		if (random()%k < 100)
		    n += 1;	/*less dramatic as things go on */
	}
    }


    /* This block seems to average out the spaces somewhat to prevent
     * cliffs and the like.
     */
#define NUM_PASSES 3
    r = 10;
    for (k=0; k<NUM_PASSES; k++) {
	for (x=2; x<max_x-2; x++) {
	    for (y=2; y<max_y - 2; y++) {
		newalt = (altitude[x][y] * r + altitude[x-1][y] +
			altitude[x][y-1] + altitude[x-1][y-1] +
			altitude[x+1][y] + altitude[x][y+1] +
			altitude[x+1][y+1] + altitude[x+1][y-1] +
			altitude[x-1][y+1]) / (r+8);
		if (altitude[x][y] < 10 || altitude[x][y] > newalt) altitude[x][y] = newalt;
	    }
	}
	for (x=max_x-2; x>2; x--) {
	    for (y=max_y-2; y>2; y--) {
		newalt = (altitude[x][y] * r + altitude[x-1][y] +
			altitude[x][y-1] + altitude[x-1][y-1] +
			altitude[x+1][y] + altitude[x][y+1] +
			altitude[x+1][y+1] + altitude[x+1][y-1] +
			altitude[x-1][y+1]) / (r+8);
		if (altitude[x][y] < 10 || altitude[x][y] > newalt) altitude[x][y] = newalt;
	    }
	}
    }

/* Make this 100 so that we eliminate/reduce the lakiness of
 * the map that is otherwise generated - otherwise, the map
 * looks like an archipelligo
 */
#define AVG_PT	-10

    /* water - does the same as above, but try to more equally balnace the spaces*/
    r = 1;
    for (k=0; k<40; k++) {
	for (x=2; x<max_x-2; x++) {
	    for (y=2; y<max_y -2; y++) {
		if (altitude[x][y] < AVG_PT)
		    altitude[x][y] = (altitude[x][y] * r + altitude[x-1][y] +
			altitude[x][y-1] + altitude[x-1][y-1] +
			altitude[x+1][y] + altitude[x][y+1] +
			altitude[x+1][y+1] + altitude[x+1][y-1] +
			altitude[x-1][y+1]) / (r+8);
	    }
	}
	for (x=max_x-2; x>2; x--) {
	    for (y=max_y-2; y>2; y--) {
		if (altitude[x][y] < AVG_PT)
		    altitude[x][y] = (altitude[x][y] * r + altitude[x-1][y] +
			altitude[x][y-1] + altitude[x-1][y-1] +
			altitude[x+1][y] + altitude[x][y+1] +
			altitude[x+1][y+1] + altitude[x+1][y-1] +
			altitude[x-1][y+1]) / (r+8);
	    }
	}
    }
    if (write_maps)
	write_crossfire_maps(max_x, max_y);

    /* Now write the data out */

	fp = fopen("lmap", "w");
	lp = fopen("pmap", "w");
	fprintf(fp, "P3 %d %d 255\n", max_y, max_x);
	for (j=0; j < max_x; j++) {
	    for (k=0; k < max_y; k++) {
		junk = altitude[j][k];
		fprintf(lp, "%d ", altitude[j][k]);
		if (junk < -5000)
		    fprintf(fp, "0 0 127 ");
		/* Shallow water should really be just at the coastal
		 * area, so put a big gap between shallow and deep.
		 * this also evens out the occurance of the different types
		 * to be more equal
		 */
		else if (junk < -99)
		    fprintf(fp, "0 0 192 ");
		else if (junk < 1)
		    fprintf(fp, "0 0 255 ");
		else if (junk < 1000)
		    fprintf(fp, "0 240 0 ");
		else if (junk < 2000)
		    fprintf(fp, "0 220 0 ");
		else if (junk < 3000)
		    fprintf(fp, "0 200 0 ");
		else if (junk < 4000)
		    fprintf(fp, "0 180 0 ");
		else if (junk < 5000)
		    fprintf(fp, "0 160 0 ");
		else if (junk < 6000)
		    fprintf(fp, "255 130 71 ");
		else if (junk < 8000)
		    fprintf(fp, "238 121 66 ");
		else if (junk < 10000)
		    fprintf(fp, "205 104  57  ");
		else if (junk < 12000)
		    fprintf(fp, "139  71  38  ");
		else
		    fprintf(fp, "255 255 255 ");
		}
	fprintf(fp, "\n");
	}
    exit(0);
}		
