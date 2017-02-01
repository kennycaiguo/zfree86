/*
 * DviChar.c
 *
 * Map DVI (ditrof output) character names to
 * font indexes and back
 */
/* $XFree86: xc/programs/xditview/DviChar.c,v 1.4 2006/01/09 15:01:02 dawes Exp $ */

# include   <X11/Intrinsic.h>
# include   "DviChar.h"

# define allocHash()	((DviCharNameHash *) XtMalloc (sizeof (DviCharNameHash)))

struct map_list {
	struct map_list	*next;
	DviCharNameMap	*map;
};

static struct map_list	*world;

static int	standard_maps_loaded = 0;
static void	load_standard_maps (void);
static int	hash_name (char *name);
static void	dispose_hash(DviCharNameMap *map);
static void	compute_hash(DviCharNameMap *map);

DviCharNameMap *
DviFindMap (encoding)
	char	*encoding;
{
	struct map_list	*m;

	if (!standard_maps_loaded)
		load_standard_maps ();
	for (m = world; m; m=m->next)
		if (!strcmp (m->map->encoding, encoding))
			return m->map;
	return 0;
}

void
DviRegisterMap (map)
	DviCharNameMap	*map;
{
	struct map_list	*m;

	if (!standard_maps_loaded)
		load_standard_maps ();
	for (m = world; m; m = m->next)
		if (!strcmp (m->map->encoding, map->encoding))
			break;
	if (!m) {
		m = (struct map_list *) XtMalloc (sizeof *m);
		m->next = world;
		world = m;
	}
	dispose_hash (map);
	m->map = map;
	compute_hash (map);
}

static void
dispose_hash (map)
	DviCharNameMap	*map;
{
	DviCharNameHash	**buckets;
	DviCharNameHash	*h, *next;
	int		i;

	buckets = map->buckets;
	for (i = 0; i < DVI_HASH_SIZE; i++) {
		for (h = buckets[i]; h; h=next) {
			next = h->next;
			XtFree ((char *) h);
		}
	}
}

static int
hash_name (name)
	char	*name;
{
	int	i = 0;

	while (*name)
		i = (i << 1) ^ *name++;
	if (i < 0)
		i = -i;
	return i;
}

static void
compute_hash (map)
	DviCharNameMap	*map;
{
	DviCharNameHash	**buckets;
	int		c, s, i;
	DviCharNameHash	*h;

	buckets = map->buckets;
	for (i = 0; i < DVI_HASH_SIZE; i++)
		buckets[i] = 0;
	for (c = 0; c < DVI_MAP_SIZE; c++)
		for (s = 0; s < DVI_MAX_SYNONYMS; s++) {
			if (!map->dvi_names[c][s])
				break;
			i = hash_name (map->dvi_names[c][s]) % DVI_HASH_SIZE;
			h = allocHash ();
			h->next = buckets[i];
			buckets[i] = h;
			h->name = map->dvi_names[c][s];
			h->position = c;
		}
	
}

int
DviCharIndex (map, name)
	DviCharNameMap	*map;
	char		*name;
{
	int		i;
	DviCharNameHash	*h;

	i = hash_name (name) % DVI_HASH_SIZE;
	for (h = map->buckets[i]; h; h=h->next)
		if (!strcmp (h->name, name))
			return h->position;
	return -1;
}

static DviCharNameMap ISO8859_1_map = {
	"iso8859-1",
	0,
{
{	0,		/* 0 */},
{	0,		/* 1 */},
{	0,		/* 2 */},
{	0,		/* 3 */},
{	0,		/* 4 */},
{	0,		/* 5 */},
{	0,		/* 6 */},
{	0,		/* 7 */},
{	0,		/* 8 */},
{	0,		/* 9 */},
{	0,		/* 10 */},
{	0,		/* 11 */},
{	0,		/* 12 */},
{	0,		/* 13 */},
{	0,		/* 14 */},
{	0,		/* 15 */},
{	0,		/* 16 */},
{	0,		/* 17 */},
{	0,		/* 18 */},
{	0,		/* 19 */},
{	0,		/* 20 */},
{	0,		/* 21 */},
{	0,		/* 22 */},
{	0,		/* 23 */},
{	0,		/* 24 */},
{	0,		/* 25 */},
{	0,		/* 26 */},
{	0,		/* 27 */},
{	0,		/* 28 */},
{	0,		/* 29 */},
{	0,		/* 30 */},
{	0,		/* 31 */},
{	0,		/* 32 */},
{	"!",		/* 33 */},
{	"\"",		/* 34 */},
{	"#",		/* 35 */},
{	"$",		/* 36 */},
{	"%",		/* 37 */},
{	"&",		/* 38 */},
{	"'",		/* 39 */},
{	"(",		/* 40 */},
{	")",		/* 41 */},
{	"*",		/* 42 */},
{	"+",		/* 43 */},
{	",",		/* 44 */},
{	"-","\\-",	/* 45 */},
{	".",		/* 46 */},
{	"/","sl",	/* 47 */},
{	"0",		/* 48 */},
{	"1",		/* 49 */},
{	"2",		/* 50 */},
{	"3",		/* 51 */},
{	"4",		/* 52 */},
{	"5",		/* 53 */},
{	"6",		/* 54 */},
{	"7",		/* 55 */},
{	"8",		/* 56 */},
{	"9",		/* 57 */},
{	":",		/* 58 */},
{	";",		/* 59 */},
{	"<",		/* 60 */},
{	"=","eq",	/* 61 */},
{	">",		/* 62 */},
{	"?",		/* 63 */},
{	"@",		/* 64 */},
{	"A",		/* 65 */},
{	"B",		/* 66 */},
{	"C",		/* 67 */},
{	"D",		/* 68 */},
{	"E",		/* 69 */},
{	"F",		/* 70 */},
{	"G",		/* 71 */},
{	"H",		/* 72 */},
{	"I",		/* 73 */},
{	"J",		/* 74 */},
{	"K",		/* 75 */},
{	"L",		/* 76 */},
{	"M",		/* 77 */},
{	"N",		/* 78 */},
{	"O",		/* 79 */},
{	"P",		/* 80 */},
{	"Q",		/* 81 */},
{	"R",		/* 82 */},
{	"S",		/* 83 */},
{	"T",		/* 84 */},
{	"U",		/* 85 */},
{	"V",		/* 86 */},
{	"W",		/* 87 */},
{	"X",		/* 88 */},
{	"Y",		/* 89 */},
{	"Z",		/* 90 */},
{	"[",		/* 91 */},
{	"\\",		/* 92 */},
{	"]",		/* 93 */},
{	"a^",		/* 94 */},
{	"_","ru","ul",	/* 95 */},
{	"`",		/* 96 */},
{	"a",		/* 97 */},
{	"b",		/* 98 */},
{	"c",		/* 99 */},
{	"d",		/* 100 */},
{	"e",		/* 101 */},
{	"f",		/* 102 */},
{	"g",		/* 103 */},
{	"h",		/* 104 */},
{	"i",		/* 105 */},
{	"j",		/* 106 */},
{	"k",		/* 107 */},
{	"l",		/* 108 */},
{	"m",		/* 109 */},
{	"n",		/* 110 */},
{	"o",		/* 111 */},
{	"p",		/* 112 */},
{	"q",		/* 113 */},
{	"r",		/* 114 */},
{	"s",		/* 115 */},
{	"t",		/* 116 */},
{	"u",		/* 117 */},
{	"v",		/* 118 */},
{	"w",		/* 119 */},
{	"x",		/* 120 */},
{	"y",		/* 121 */},
{	"z",		/* 122 */},
{	"{",		/* 123 */},
{	"|","or"	/* 124 */},
{	"}",		/* 125 */},
{	"a~","ap"	/* 126 */},
{	0,		/* 127 */},
{	0,		/* 128 */},
{	0,		/* 129 */},
{	0,		/* 130 */},
{	0,		/* 131 */},
{	0,		/* 132 */},
{	0,		/* 133 */},
{	0,		/* 134 */},
{	0,		/* 135 */},
{	0,		/* 136 */},
{	0,		/* 137 */},
{	0,		/* 138 */},
{	0,		/* 139 */},
{	0,		/* 140 */},
{	0,		/* 141 */},
{	0,		/* 142 */},
{	0,		/* 143 */},
{	0,		/* 144 */},
{	0,		/* 145 */},
{	0,		/* 146 */},
{	0,		/* 147 */},
{	0,		/* 148 */},
{	0,		/* 149 */},
{	0,		/* 150 */},
{	0,		/* 151 */},
{	0,		/* 152 */},
{	0,		/* 153 */},
{	0,		/* 154 */},
{	0,		/* 155 */},
{	0,		/* 156 */},
{	0,		/* 157 */},
{	0,		/* 158 */},
{	0,		/* 159 */},
{	0,		/* 160 */},
{	"I!",		/* 161 */},
{	"ct",		/* 162 */},
{	"po",		/* 163 */},
{	"cu",		/* 164 */},
{	"$J",		/* 165 */},
{	0,		/* 166 */},
{	"sc",		/* 167 */},
{	0,		/* 168 */},
{	"co",		/* 169 */},
{	0,		/* 170 */},
{	"d<",		/* 171 */},
{	"no",		/* 172 */},
{	"hy",		/* 173 */},
{	"rg",		/* 174 */},
{	"ma",		/* 175 */},
{	"de",		/* 176 */},
{	"+-",		/* 177 */},
{	0,		/* 178 */},
{	0,		/* 179 */},
{	"aa",		/* 180 */},
{	"*m",		/* 181 */},
{	"pp",		/* 182 */},
{	0,		/* 183 */},
{	"cd",		/* 184 */},
{	0,		/* 185 */},
{	0,		/* 186 */},
{	"d>",		/* 187 */},
{	"14",		/* 188 */},
{	"12",		/* 189 */},
{	"34",		/* 190 */},
{	"I?",		/* 191 */},
{	0,		/* 192 */},
{	0,		/* 193 */},
{	0,		/* 194 */},
{	0,		/* 195 */},
{	0,		/* 196 */},
{	0,		/* 197 */},
{	"AE",		/* 198 */},
{	0,		/* 199 */},
{	0,		/* 200 */},
{	0,		/* 201 */},
{	0,		/* 202 */},
{	0,		/* 203 */},
{	0,		/* 204 */},
{	0,		/* 205 */},
{	0,		/* 206 */},
{	0,		/* 207 */},
{	0,		/* 208 */},
{	0,		/* 209 */},
{	0,		/* 210 */},
{	0,		/* 211 */},
{	0,		/* 212 */},
{	0,		/* 213 */},
{	0,		/* 214 */},
{	"mu",		/* 215 */},
{	"O/",		/* 216 */},
{	0,		/* 217 */},
{	0,		/* 218 */},
{	0,		/* 219 */},
{	0,		/* 220 */},
{	0,		/* 221 */},
{	0,		/* 222 */},
{	"ss",		/* 223 */},
{	0,		/* 224 */},
{	0,		/* 225 */},
{	0,		/* 226 */},
{	0,		/* 227 */},
{	0,		/* 228 */},
{	0,		/* 229 */},
{	"ae",		/* 230 */},
{	0,		/* 231 */},
{	0,		/* 232 */},
{	0,		/* 233 */},
{	0,		/* 234 */},
{	0,		/* 235 */},
{	0,		/* 236 */},
{	0,		/* 237 */},
{	0,		/* 238 */},
{	0,		/* 239 */},
{	0,		/* 240 */},
{	0,		/* 241 */},
{	0,		/* 242 */},
{	0,		/* 243 */},
{	0,		/* 244 */},
{	0,		/* 245 */},
{	0,		/* 246 */},
{	0,		/* 247 */},
{	0,		/* 248 */},
{	0,		/* 249 */},
{	0,		/* 250 */},
{	0,		/* 251 */},
{	0,		/* 252 */},
{	0,		/* 253 */},
{	0,		/* 254 */},
{	0,		/* 255 */},
},
{
{   "fi",	    "fi",   },
{   "fl",	    "fl",   },
{   "ff",	    "ff",   },
{   "Fi",	    "ffi",   },
{   "Fl",	    "ffl",   },
{   0,		    0,	    },
}
};

static DviCharNameMap Adobe_Symbol_map = {
	"adobe-fontspecific",
	1,
{
{	0,		/* 0 */},
{	0,		/* 1 */},
{	0,		/* 2 */},
{	0,		/* 3 */},
{	0,		/* 4 */},
{	0,		/* 5 */},
{	0,		/* 6 */},
{	0,		/* 7 */},
{	0,		/* 8 */},
{	0,		/* 9 */},
{	0,		/* 10 */},
{	0,		/* 11 */},
{	0,		/* 12 */},
{	0,		/* 13 */},
{	0,		/* 14 */},
{	0,		/* 15 */},
{	0,		/* 16 */},
{	0,		/* 17 */},
{	0,		/* 18 */},
{	0,		/* 19 */},
{	0,		/* 20 */},
{	0,		/* 21 */},
{	0,		/* 22 */},
{	0,		/* 23 */},
{	0,		/* 24 */},
{	0,		/* 25 */},
{	0,		/* 26 */},
{	0,		/* 27 */},
{	0,		/* 28 */},
{	0,		/* 29 */},
{	0,		/* 30 */},
{	0,		/* 31 */},
{	0,		/* 32 */},
{	"!",		/* 33 */},
{	"fa",		/* 34 */},
{	"#",		/* 35 */},
{	"te",		/* 36 */},
{	"%",		/* 37 */},
{	"&",		/* 38 */},
{	"cm",		/* 39 */},
{	"(",		/* 40 */},
{	")",		/* 41 */},
{	"**",		/* 42 */},
{	"+","pl",	/* 43 */},
{	",",		/* 44 */},
{	"-","mi","\\-",	/* 45 */},
{	".",		/* 46 */},
{	"/","sl",	/* 47 */},
{	"0",		/* 48 */},
{	"1",		/* 49 */},
{	"2",		/* 50 */},
{	"3",		/* 51 */},
{	"4",		/* 52 */},
{	"5",		/* 53 */},
{	"6",		/* 54 */},
{	"7",		/* 55 */},
{	"8",		/* 56 */},
{	"9",		/* 57 */},
{	":",		/* 58 */},
{	";",		/* 59 */},
{	"<",		/* 60 */},
{	"=","eq",	/* 61 */},
{	">",		/* 62 */},
{	"?",		/* 63 */},
{	"=~",		/* 64 */},
{	"*A",		/* 65 */},
{	"*B",		/* 66 */},
{	"*C",		/* 67 */},
{	"*D",		/* 68 */},
{	"*E",		/* 69 */},
{	"*F",		/* 70 */},
{	"*G",		/* 71 */},
{	"*Y",		/* 72 */},
{	"*I",		/* 73 */},
{	0,		/* 74 */},
{	"*K",		/* 75 */},
{	"*L",		/* 76 */},
{	"*M",		/* 77 */},
{	"*N",		/* 78 */},
{	"*O",		/* 79 */},
{	"*P",		/* 80 */},
{	"*H",		/* 81 */},
{	"*R",		/* 82 */},
{	"*S",		/* 83 */},
{	"*T",		/* 84 */},
{	"*U",		/* 85 */},
{	"ts",		/* 86 */},
{	"*W",		/* 87 */},
{	"*C",		/* 88 */},
{	"*Q",		/* 89 */},
{	"*Z",		/* 90 */},
{	"[",		/* 91 */},
{	"tf",		/* 92 */},
{	"]",		/* 93 */},
{	"bt",		/* 94 */},
{	"ul",		/* 95 */},
{	"rn",		/* 96 */},
{	"*a",		/* 97 */},
{	"*b",		/* 98 */},
{	"*x",		/* 99 */},
{	"*d",		/* 100 */},
{	"*e",		/* 101 */},
{	"*f",		/* 102 */},
{	"*g",		/* 103 */},
{	"*y",		/* 104 */},
{	"*i",		/* 105 */},
{	0,		/* 106 */},
{	"*k",		/* 107 */},
{	"*l",		/* 108 */},
{	"*m",		/* 109 */},
{	"*n",		/* 110 */},
{	"*o",		/* 111 */},
{	"*p",		/* 112 */},
{	"*h",		/* 113 */},
{	"*r",		/* 114 */},
{	"*s",		/* 115 */},
{	"*t",		/* 116 */},
{	"*u",		/* 117 */},
{	0,		/* 118 */},
{	"*w",		/* 119 */},
{	"*c",		/* 120 */},
{	"*q",		/* 121 */},
{	"*z",		/* 122 */},
{	"{",		/* 123 */},
{	"|",		/* 124 */},
{	"}",		/* 125 */},
{	"a~","ap",	/* 126 */},
{	0,		/* 127 */},
{	0,		/* 128 */},
{	0,		/* 129 */},
{	0,		/* 130 */},
{	0,		/* 131 */},
{	0,		/* 132 */},
{	0,		/* 133 */},
{	0,		/* 134 */},
{	0,		/* 135 */},
{	0,		/* 136 */},
{	0,		/* 137 */},
{	0,		/* 138 */},
{	0,		/* 139 */},
{	0,		/* 140 */},
{	0,		/* 141 */},
{	0,		/* 142 */},
{	0,		/* 143 */},
{	0,		/* 144 */},
{	0,		/* 145 */},
{	0,		/* 146 */},
{	0,		/* 147 */},
{	0,		/* 148 */},
{	0,		/* 149 */},
{	0,		/* 150 */},
{	0,		/* 151 */},
{	0,		/* 152 */},
{	0,		/* 153 */},
{	0,		/* 154 */},
{	0,		/* 155 */},
{	0,		/* 156 */},
{	0,		/* 157 */},
{	0,		/* 158 */},
{	0,		/* 159 */},
{	0,		/* 160 */},
{	0,		/* 161 */},
{	"fm","mt",	/* 162 */},
{	"<=",		/* 163 */},
{	"/","sl"	/* 164 */},
{	"if",		/* 165 */},
{	0,		/* 166 */},
{	"Cc",		/* 167 */},
{	"Cd",		/* 168 */},
{	"Ch",		/* 169 */},
{	"Cs",		/* 170 */},
{	"<>",		/* 171 */},
{	"<-",		/* 172 */},
{	"ua",		/* 173 */},
{	"->",		/* 174 */},
{	"da",		/* 175 */},
{	"de",		/* 176 */},
{	"+-",		/* 177 */},
{	"sd",		/* 178 */},
{	">=",		/* 179 */},
{	"mu",		/* 180 */},
{	"pt",		/* 181 */},
{	"pd",		/* 182 */},
{	"bu",		/* 183 */},
{	"di",		/* 184 */},
{	"!=",		/* 185 */},
{	"==",		/* 186 */},
{	"~=",		/* 187 */},
{	"..",		/* 188 */},
{	"br",		/* 189 */},
{	"em",		/* 190 */},
{	"cr",		/* 191 */},
{	"al",		/* 192 */},
{	0,		/* 193 */},
{	0,		/* 194 */},
{	0,		/* 195 */},
{	"ax",		/* 196 */},
{	"a+",		/* 197 */},
{	"es",		/* 198 */},
{	"ca",		/* 199 */},
{	"cu",		/* 200 */},
{	"sp",		/* 201 */},
{	"ip",		/* 202 */},
{	"!s",		/* 203 */},
{	"sb",		/* 204 */},
{	"ib",		/* 205 */},
{	"mo",		/* 206 */},
{	"!m",		/* 207 */},
{	"ag",		/* 208 */},
{	"gr",		/* 209 */},
{	0,		/* 210 */},
{	0,		/* 211 */},
{	0,		/* 212 */},
{	0,		/* 213 */},
{	"sr",		/* 214 */},
{	"m.",		/* 215 */},
{	"no",		/* 216 */},
{	"an",		/* 217 */},
{	"lo",		/* 218 */},
{	"io",		/* 219 */},
{	"<:",		/* 220 */},
{	"u=",		/* 221 */},
{	":>",		/* 222 */},
{	"d=",		/* 223 */},
{	"dm",		/* 224 */},
{	"L<",		/* 225 */},
{	"rg",		/* 226 */},
{	"co",		/* 227 */},
{	"tm",		/* 228 */},
{	0,		/* 229 */},
{	0,		/* 230 */},
{	0,		/* 231 */},
{	0,		/* 232 */},
{	"lc",		/* 233 */},
{	0,		/* 234 */},
{	"lf",		/* 235 */},
{	"lt",		/* 236 */},
{	"lk",		/* 237 */},
{	"lb",		/* 238 */},
{	"bv",		/* 239 */},
{	"AL",		/* 240 */},
{	"R>",		/* 241 */},
{	"is",		/* 242 */},
{	0,		/* 243 */},
{	0,		/* 244 */},
{	0,		/* 245 */},
{	0,		/* 246 */},
{	0,		/* 247 */},
{	0,		/* 248 */},
{	"rc",		/* 249 */},
{	0,		/* 250 */},
{	"rf",		/* 251 */},
{	"rt",		/* 252 */},
{	"rk",		/* 253 */},
{	"rb",		/* 254 */},
{	0,		/* 255 */},
},
{
{	0,	    0	},
}
};

static void
load_standard_maps ()
{
	standard_maps_loaded = 1;
	DviRegisterMap (&ISO8859_1_map);
	DviRegisterMap (&Adobe_Symbol_map);
}

