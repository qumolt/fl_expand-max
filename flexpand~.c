#include "flexpand~.h"

void ext_main(void *r)
{
	t_class *c = class_new("flexpand~", (method)fl_expand_new, (method)fl_expand_free, sizeof(t_fl_expand), 0L, A_GIMME, 0);

	class_addmethod(c, (method)fl_expand_dsp64, "dsp64", A_CANT, 0);
	class_addmethod(c, (method)fl_expand_set, "set", A_GIMME, 0);
	class_addmethod(c, (method)fl_expand_assist, "assist", A_CANT, 0);
	class_addmethod(c, (method)fl_expand_dblclick, "dblclick", A_CANT, 0);
	class_addmethod(c, (method)fl_expand_notify, "notify", A_CANT, 0);

	class_addmethod(c, (method)fl_expand_bang, "bang", 0);
	class_addmethod(c, (method)fl_expand_canal, "chan", A_GIMME, 0);
	class_addmethod(c, (method)fl_expand_divisiones, "ft4", A_FLOAT, 0);
	class_addmethod(c, (method)fl_expand_inicio, "ft3", A_FLOAT, 0);
	class_addmethod(c, (method)fl_expand_delta, "ft2", A_FLOAT, 0);
	class_addmethod(c, (method)fl_expand_duracion, "ft1", A_FLOAT, 0);

	class_addmethod(c, (method)fl_expand_samps_limits, "samplim", A_GIMME, 0);

	class_dspinit(c);
	class_register(CLASS_BOX, c);
	fl_expand_class = c;

	return;
}

/* new and assist --------------------------------------------------------------------*/
void *fl_expand_new(t_symbol *s, short argc, t_atom* argv)
{
	t_fl_expand *x = (t_fl_expand *)object_alloc(fl_expand_class);

	/* division(4), start(3), end(2), duration beats(1) */
	floatin(x, 1);
	floatin(x, 2);
	floatin(x, 3);
	floatin(x, 4);

	dsp_setup((t_pxobject *)x, 1);	//fasor
	outlet_new((t_object *)x, "signal");
	outlet_new((t_object *)x, "signal");

	x->divs = 1.f;
	x->dur_beats = 0.f;
	x->ini_beats = 0.f;
	x->delta_beats = 1.f;

	x->l_chan_sel = 0;

	return (x);
}

void fl_expand_assist(t_fl_expand *x, void *b, long msg, long arg, char *dst)
{
	if (msg == ASSIST_INLET) {
		switch (arg) {
		case I_FASOR: sprintf(dst, "(sig~) nomalized phasor [0, 1]"); break;
		case I_DIVISIONES: sprintf(dst, "(float) n divisions (beats)"); break;
		case I_INICIO: sprintf(dst, "(float) start (beats)"); break;
		case I_DELTA: sprintf(dst, "(float) delta (beats)"); break;
		case I_DURACION: sprintf(dst, "(float) total time to play (beats)"); break;
		}
	}
	else if (msg == ASSIST_OUTLET) {
		switch (arg) {
		case O_AUDIO: sprintf(dst, "(sig~) audio output"); break;
		case O_FLAG: sprintf(dst, "(bang) final flag"); break;
		}
	}
}

/* memory ---------------------------------------------------------------------------------*/
void fl_expand_free(t_fl_expand *x)
{
	dsp_free((t_pxobject *)x);
	
	object_free(x->l_buffer_reference);
}

/* specific -------------------------------------------------------------------------------*/
void fl_expand_bang(t_fl_expand *x) 
{
	t_buffer_obj *buffer;
	float *tab;

	if (!x->loaded_buf) { object_warn((t_object *)x, "no buffer loaded"); return; }
	if (!x->l_buffer_reference) { object_warn((t_object *)x, "couldn't find buffer reference"); return; }

	/* secure buffer */
	buffer = buffer_ref_getobject(x->l_buffer_reference);
	if (!buffer) { object_error((t_object *)x, "couldn't find buffer"); return; }
	tab = buffer_locksamples(buffer);
	if (!tab) { object_error((t_object *)x, "blank buffer"); return; }

	long len = (long)buffer_getframecount(buffer);
	long nc = (long)buffer_getchannelcount(buffer);
	float source_sr = (float)buffer_getsamplerate(buffer);
	long chan = 0;
	long samp_ini = 0;
	long samp_fin = len - 1;

	buffer_unlocksamples(buffer);

	x->source_len = len;
	x->l_chan_sel = MIN(chan, nc - 1);
	x->samp_ini = samp_ini;
	x->samp_fin = samp_fin;

	object_post((t_object *)x, "buffer: \nchannels: %d \nselected chan: %d \nlength(smps): %d \nlimits set to: [0, %d]", nc, chan, len, samp_fin);
}
void fl_expand_canal(t_fl_expand *x, t_symbol *s, long argc, t_atom *argv)
{
	long n;
	if (argc != 1) { object_warn((t_object *)x, "one value: channel number"); return; }
	if (atom_gettype(argv) != A_FLOAT && atom_gettype(argv) != A_LONG) { object_warn((t_object *)x, "value must be a number"); return; }
	n = (long)atom_getlong(argv);
	x->l_chan_sel = MAX(n, 1) - 1;
}

void fl_expand_divisiones(t_fl_expand *x, double f)
{
	if (f < 1.0) { object_warn((t_object *)x, "divisions must be a positive equal or more than 1"); return; }
	x->divs = (float)f;
}
void fl_expand_duracion(t_fl_expand *x, double f)
{
	if (f < 0.0) { object_warn((t_object *)x, "duration must be 0 or positive"); return; }
	x->dur_beats = (float)f;
}
void fl_expand_inicio(t_fl_expand *x, double f)
{
	if (f < 0.0) { object_warn((t_object *)x, "start must be 0 or positive"); return; }
	x->ini_beats = (float)f;
}
void fl_expand_delta(t_fl_expand *x, double f)
{
	if (f < 0.1) { object_warn((t_object *)x, "delta must be equal or more than 0.1"); return; }
	x->delta_beats = MAX(DELTA_MIN, (float)f);
}

/* messages -------------------------------------------------------------------------------*/
void fl_expand_samps_limits(t_fl_expand *x, t_symbol *s, long argc, t_atom *argv)
{
	long start_samp, end_samp, source_len;
	if (argc != 2) { object_warn((t_object *)x, "two values: start sample, end sample"); return; }
	if (atom_gettype(argv) != A_FLOAT && atom_gettype(argv) != A_LONG && atom_gettype(argv+1) != A_FLOAT && atom_gettype(argv+1) != A_LONG) {
		object_warn((t_object *)x, "value must be a number"); return; 
	}
	if (!x->loaded_buf) { object_warn((t_object *)x, "load a buffer"); return; }

	source_len = x->source_len;
	start_samp = (long)atom_getlong(argv);
	end_samp = (long)atom_getlong(argv + 1);

	//parsed on perform64()

	x->samp_ini = start_samp;
	x->samp_fin = end_samp;
}

/* buffer ---------------------------------------------------------------------------------*/
t_max_err fl_expand_notify(t_fl_expand *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
	return buffer_ref_notify(x->l_buffer_reference, s, msg, sender, data);
}

void fl_expand_set(t_fl_expand *x, t_symbol *s, long argc, t_atom *argv)
{
	t_buffer_obj *buffer;
	float *tab;
	long ac = argc;
	t_atom *av = argv;
	t_symbol *name;

	if (!ac || (ac != 1 && ac != 2 && ac != 4)) { object_warn((t_object *)x, "arguments must be: \nbuf name \nbuf name, chan \nbuf name, chan, start samp, final samp"); return; }
	if (ac) { if (atom_gettype(av) != A_SYM) { object_warn((t_object *)x, "buffer name must be a string"); return; } }
	if (ac == 2) { if (atom_gettype(av + 1) != A_FLOAT && atom_gettype(av + 1) != A_LONG) { object_warn((t_object *)x, "channel must be number"); return; } }
	if (ac == 4) {
		if ((atom_gettype(av + 2) != A_FLOAT && atom_gettype(av + 2) != A_LONG) ||
			(atom_gettype(av + 3) != A_FLOAT && atom_gettype(av + 3) != A_LONG)) {
			object_warn((t_object *)x, "sample bounds must be numbers"); return;
		}
	}

	name = atom_getsym(av);

	if (!x->l_buffer_reference) { x->l_buffer_reference = buffer_ref_new((t_object *)x, name); }
	else { buffer_ref_set(x->l_buffer_reference, name); }

	/* secure buffer */
	buffer = buffer_ref_getobject(x->l_buffer_reference);
	if (!buffer) { object_error((t_object *)x, "couldn't find buffer"); return; }
	tab = buffer_locksamples(buffer);
	if (!tab) { object_error((t_object *)x, "blank buffer"); return; }

	long len = (long)buffer_getframecount(buffer);
	long nc = (long)buffer_getchannelcount(buffer);
	float source_sr = (float)buffer_getsamplerate(buffer);
	long chan = 0;
	long samp_ini = 0;
	long samp_fin = len - 1;

	buffer_unlocksamples(buffer);

	if (ac == 4) {
		chan = (long)atom_getlong(av + 1);
		samp_ini = (long)atom_getlong(av + 2);
		samp_fin = (long)atom_getlong(av + 3);
	}
	else if (ac == 2) {
		chan = (long)atom_getlong(av + 1);
		x->samp_ini = 0;
		x->samp_fin = len - 1;
	}
	else {
		chan = 1;
		x->samp_ini = 0;
		x->samp_fin = len - 1;
	}

	x->source_len = len;
	x->l_chan_sel = MIN(chan, nc - 1);
	x->samp_ini = samp_ini; //min and max values are parsed on perform64
	x->samp_fin = samp_fin; //parsing is done later in case buffer changed
	x->loaded_buf = 1;
	object_post((t_object *)x, "buffer: %s \nchannels: %d \nselected chan: %d \nlength(smps): %d", name->s_name, nc, chan, len);
}

void fl_expand_dblclick(t_fl_expand *x)
{
	buffer_view(buffer_ref_getobject(x->l_buffer_reference));
}

/*audio -----------------------------------------------------------------------------------*/
void fl_expand_dsp64(t_fl_expand *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	x->fasor_connected = count[0];
	
	if (x->sr != (long)samplerate) {
		x->sr = (long)samplerate;
	}

	dsp_add64(dsp64, (t_object *)x, (t_perfroutine64)fl_expand_perform64, 0, NULL);
}
void fl_expand_perform64(t_fl_expand *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	t_double *in = ins[0];
	t_double *out1 = outs[0];
	t_double *out2 = outs[1];
	int	n = sampleframes;
	t_float	*tab;
	t_buffer_obj *buffer = buffer_ref_getobject(x->l_buffer_reference);

	double outsamp;

	tab = buffer_locksamples(buffer);
	if (!tab) {
		x->loaded_buf = 0;
		goto zero;
	}

	long len = x->source_len = (long)buffer_getframecount(buffer);
	long nc = (long)buffer_getchannelcount(buffer);
	float source_sr = (float)buffer_getsamplerate(buffer);
	long chan = MIN(x->l_chan_sel, nc - 1);
	
	double fasor;
	double findex;
	long index;
	float dur = x->dur_beats;
	float div = x->divs;
	long fin = MAX(MIN(x->samp_fin, len - 1), 0);
	long ini = MAX(MIN(x->samp_ini, len - 1), 0);
	float beat_i = (float)fmod(MAX(x->ini_beats, 0.f), div);
	float beat_d = MIN(MAX(x->delta_beats, DELTA_MIN), div - beat_i);
	double fasbar;
	long ifasbar;
	double outbar;

	while (n--) {

		outsamp = 0.0;
		outbar = 0.0;

		fasor = *in++; 

		if (x->fasor_connected) {
			
			fasor = MIN(1., MAX(0., fasor));
			findex = fasor * dur / div;
			findex = findex + beat_i / div;

			while (findex < beat_i / div) {
				findex += beat_d / div;
			}
			while (findex > (beat_i + beat_d) / div) {
				findex -= beat_d / div;
			}

			index = (long)(findex * abs(fin - ini) + ini);
			outsamp = tab[index * nc + chan];

			fasbar = fasor * div;
			ifasbar = (long)fasbar;
			outbar = (fasbar - (double)ifasbar);
		}

		*out1++ = outsamp;
		*out2++ = outbar;
	}

	buffer_unlocksamples(buffer);

	return;

zero:
	while (n--) {
		*out1++ = 0.0;
		*out2++ = 0.0;
	}
}
