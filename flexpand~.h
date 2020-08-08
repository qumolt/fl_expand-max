#include "ext.h"
#include "ext_obex.h"
#include "ext_common.h"
#include "z_dsp.h"
#include "ext_buffer.h"

#define TEMPO_DFLT 24000
#define DELTA_MIN 0.1f

enum INLETS { I_FASOR, I_DIVISIONES, I_INICIO, I_DELTA, I_DURACION, NUM_INLETS };
enum OUTLETS { O_AUDIO, O_FLAG, NUM_OUTLETS };

/* object */
typedef struct _fl_expand {
	t_pxobject l_obj;
	t_buffer_ref *l_buffer_reference;
	long l_chan_sel;

	long sr;

	short fasor_connected;

	long samp_ini;
	long samp_fin;
	short loaded_buf;
	long source_len;

	float divs;
	float dur_beats;
	float ini_beats;
	float delta_beats;

} t_fl_expand;

/* methods */
	/* Max */
void *fl_expand_new(t_symbol *s, short argc, t_atom *argv);
void fl_expand_assist(t_fl_expand *x, void *b, long m, long a, char *s);

	/* memory */
void fl_expand_free(t_fl_expand *x);

	/* object specific */
void fl_expand_bang(t_fl_expand *x);
void fl_expand_canal(t_fl_expand *x, t_symbol *s, long argc, t_atom *argv);
void fl_expand_divisiones(t_fl_expand *x, double f);
void fl_expand_duracion(t_fl_expand *x, double f);
void fl_expand_inicio(t_fl_expand *x, double f);
void fl_expand_delta(t_fl_expand *x, double f);

	/* messages */
void fl_expand_samps_limits(t_fl_expand *x, t_symbol *s, long argc, t_atom *argv);

	/* audio */
void fl_expand_dsp64(t_fl_expand *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void fl_expand_perform64(t_fl_expand *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

	/* buffer */
void fl_expand_set(t_fl_expand *x, t_symbol *s, long argc, t_atom *argv);
t_max_err fl_expand_notify(t_fl_expand *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
void fl_expand_dblclick(t_fl_expand *x);


/* class */
static t_class *fl_expand_class;