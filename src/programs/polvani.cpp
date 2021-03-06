/***********
 * WARNING:
 * This is only a sketch of the Polvani initial conditions.
 *
 * These conditions are not setup correctly!
 */


#if !SWEET_USE_SPECTRAL_SPACE
	#error "Spectral space not activated"
#endif

#include <sweet/DataArray.hpp>
#if SWEET_GUI
	#include "sweet/VisSweet.hpp"
#endif

#include <sweet/SimulationVariables.hpp>
#include <sweet/TimesteppingRK.hpp>
#include <sweet/SWEValidationBenchmarks.hpp>
#include <sweet/Operators2D.hpp>
#include <sweet/Stopwatch.hpp>

#include <sweetmath.hpp>
#include <ostream>
#include <sstream>
#include <unistd.h>
#include <iomanip>
#include <stdio.h>


SimulationVariables simVars;



DataArray<2> *init_h = nullptr;
DataArray<2> *init_u = nullptr;
DataArray<2> *init_v = nullptr;


class SimulationSWE
{
public:
	DataArray<2> prog_h, prog_u, prog_v;
	DataArray<2> eta;
	DataArray<2> tmp;

	Operators2D op;

	TimesteppingRK timestepping;

	int last_timestep_nr_update_diagnostics = -1;

	double benchmark_diff_h;
	double benchmark_diff_u;
	double benchmark_diff_v;

public:
	SimulationSWE()	:
		prog_h(simVars.disc.res),
		prog_u(simVars.disc.res),
		prog_v(simVars.disc.res),

		eta(simVars.disc.res),
		tmp(simVars.disc.res),

		op(simVars.disc.res, simVars.sim.domain_size, simVars.disc.use_spectral_basis_diffs)
	{
		reset();
	}


	void reset()
	{
		last_timestep_nr_update_diagnostics = -1;

		benchmark_diff_h = 0;
		benchmark_diff_u = 0;
		benchmark_diff_v = 0;

		simVars.timecontrol.current_timestep_nr = 0;
		simVars.timecontrol.current_simulation_time = 0;

		if (init_h != nullptr)
		{
			prog_h = *init_h;
			prog_u = *init_u;
			prog_v = *init_v;
			return;
		}

		prog_h.set_all(simVars.setup.h0);
		prog_u.set_all(0);
		prog_v.set_all(0);

		for (std::size_t j = 0; j < simVars.disc.res[1]; j++)
		{
			for (std::size_t i = 0; i < simVars.disc.res[0]; i++)
			{
				double x = (((double)i+0.5)/(double)simVars.disc.res[0])*simVars.sim.domain_size[0];
				double y = (((double)j+0.5)/(double)simVars.disc.res[1])*simVars.sim.domain_size[1];

				prog_h.set(j, i, SWEValidationBenchmarks::return_h(simVars, x, y));
				prog_u.set(j, i, SWEValidationBenchmarks::return_u(simVars, x, y));
				prog_v.set(j, i, SWEValidationBenchmarks::return_v(simVars, x, y));
			}
		}

	}



	void update_diagnostics()
	{

		// assure, that the diagnostics are only updated for new time steps
		if (last_timestep_nr_update_diagnostics == simVars.timecontrol.current_timestep_nr)
			return;

		last_timestep_nr_update_diagnostics = simVars.timecontrol.current_timestep_nr;

		double normalization = (simVars.sim.domain_size[0]*simVars.sim.domain_size[1]) /
								((double)simVars.disc.res[0]*(double)simVars.disc.res[1]);

		// mass
		simVars.diag.total_mass = prog_h.reduce_sum_quad() * normalization;

		// energy
		simVars.diag.total_energy = 0.5*(
				prog_h*prog_h +
				prog_h*prog_u*prog_u +
				prog_h*prog_v*prog_v
			).reduce_sum_quad() * normalization;

		// potential enstropy
		eta = (op.diff_c_x(prog_v) - op.diff_c_y(prog_u) + simVars.sim.f0) / prog_h;
		simVars.diag.total_potential_enstrophy = 0.5*(eta*eta*prog_h).reduce_sum_quad() * normalization;
	}



	void compute_upwinding_P_updates(
			const DataArray<2> &i_P,		///< prognostic variables (at T=tn)
			const DataArray<2> &i_u,		///< prognostic variables (at T=tn+dt)
			const DataArray<2> &i_v,		///< prognostic variables (at T=tn+dt)

			DataArray<2> &o_P_t				///< time updates (at T=tn+dt)
	)
	{
		std::cerr << "TODO: implement, is this really possible for non-staggered grid? (averaging of velocities required)" << std::endl;
		exit(-1);
		//             |                       |                       |
		// --v---------|-----------v-----------|-----------v-----------|
		//   h-1       u0          h0          u1          h1          u2
		//

		// same a above, but formulated in a finite-difference style
		o_P_t =
			(
				(
					// u is positive
					op.shift_right(i_P)*i_u.return_value_if_positive()	// inflow
					-i_P*op.shift_left(i_u.return_value_if_positive())					// outflow

					// u is negative
					+(i_P*i_u.return_value_if_negative())	// outflow
					-op.shift_left(i_P*i_u.return_value_if_negative())		// inflow
				)*(1.0/simVars.disc.cell_size[0])	// here we see a finite-difference-like formulation
				+
				(
					// v is positive
					op.shift_up(i_P)*i_v.return_value_if_positive()		// inflow
					-i_P*op.shift_down(i_v.return_value_if_positive())					// outflow

					// v is negative
					+(i_P*i_v.return_value_if_negative())	// outflow
					-op.shift_down(i_P*i_v.return_value_if_negative())	// inflow
				)*(1.0/simVars.disc.cell_size[1])
			);
	}



	void p_run_euler_timestep_update(
			const DataArray<2> &i_h,	///< prognostic variables
			const DataArray<2> &i_u,	///< prognostic variables
			const DataArray<2> &i_v,	///< prognostic variables

			DataArray<2> &o_h_t,	///< time updates
			DataArray<2> &o_u_t,	///< time updates
			DataArray<2> &o_v_t,	///< time updates

			double &o_dt,			///< time step restriction
			double i_fixed_dt = 0,		///< if this value is not equal to 0, use this time step size instead of computing one
			double i_simulation_time = -1
	)
	{
		/*
		 * non-conservative formulation:
		 *
		 *	h_t = -(u*h)_x - (v*h)_y
		 *	u_t = -g * h_x - u * u_x - v * u_y + f*v
		 *	v_t = -g * h_y - u * v_x - v * v_y - f*u
		 */
		o_u_t = -simVars.sim.g*op.diff_c_x(i_h) - i_u*op.diff_c_x(i_u) - i_v*op.diff_c_y(i_u) + simVars.sim.f0*i_v;
		o_v_t = -simVars.sim.g*op.diff_c_y(i_h) - i_u*op.diff_c_x(i_v) - i_v*op.diff_c_y(i_v) - simVars.sim.f0*i_u;

		if (simVars.sim.viscosity != 0)
		{
			o_u_t -= op.diffN_x(i_u, simVars.sim.viscosity_order)*simVars.sim.viscosity;
			o_v_t -= op.diffN_y(i_v, simVars.sim.viscosity_order)*simVars.sim.viscosity;
		}



		/*
		 * TIME STEP SIZE
		 */
		if (i_fixed_dt > 0)
		{
			o_dt = i_fixed_dt;
		}
		else
		{
			/*
			 * If the timestep size parameter is negative, we use the absolute value of this one as the time step size
			 */
			if (i_fixed_dt < 0)
			{
				o_dt = -i_fixed_dt;
			}
			else
			{
				double limit_speed = std::max(simVars.disc.cell_size[0]/i_u.reduce_maxAbs(), simVars.disc.cell_size[1]/i_v.reduce_maxAbs());

				// limit by re
				double limit_visc = std::numeric_limits<double>::infinity();
		//        if (viscocity > 0)
		//           limit_visc = (viscocity*0.5)*((hx*hy)*0.5);

				// limit by gravitational acceleration
				double limit_gh = std::min(simVars.disc.cell_size[0], simVars.disc.cell_size[1])/std::sqrt(simVars.sim.g*i_h.reduce_maxAbs());

				if (simVars.misc.verbosity > 2)
					std::cerr << "limit_speed: " << limit_speed << ", limit_visc: " << limit_visc << ", limit_gh: " << limit_gh << std::endl;

				o_dt = simVars.sim.CFL*std::min(std::min(limit_speed, limit_visc), limit_gh);
			}
		}

		if (!simVars.disc.timestepping_leapfrog_like_update)
		{
			if (!simVars.disc.timestepping_up_and_downwinding)
			{
				// standard update
				o_h_t = -op.diff_c_x(i_u*i_h) - op.diff_c_y(i_v*i_h);
			}
			else
			{
				// up/down winding
				compute_upwinding_P_updates(
						i_h,
						i_u,
						i_v,
						o_h_t
					);
			}
		}
		else
		{
			/*
			 * a kind of leapfrog:
			 *
			 * We use the hew v and u values to compute the update for p
			 *
			 * compute updated u and v values without using it
			 */
			if (!simVars.disc.timestepping_up_and_downwinding)
			{
				// recompute U and V

				// update based on new u and v values
				o_h_t = -op.diff_c_x(
							i_h*(i_u+o_dt*o_u_t)
						)
						- op.diff_c_y(
							i_h*(i_v+o_dt*o_v_t)
						);
			}
			else
			{
				// update based on new u and v values
				compute_upwinding_P_updates(
						i_h,
						i_u+o_dt*o_u_t,
						i_v+o_dt*o_v_t,
						o_h_t
					);
			}
		}
#if 0
		if (simVars.sim.potential_viscosity != 0)
			o_h_t -= op.diff2(i_h)*simVars.sim.potential_viscosity;

		if (simVars.sim.potential_hyper_viscosity != 0)
			o_h_t -= op.diff4(i_h)*simVars.sim.potential_hyper_viscosity;
#endif
	}



	void run_timestep()
	{
		double dt;

		// either set time step size to 0 for autodetection or to
		// a positive value to use a fixed time step size
		simVars.timecontrol.current_timestep_size = (simVars.sim.CFL < 0 ? -simVars.sim.CFL : 0);

		timestepping.run_rk_timestep(
				this,
				&SimulationSWE::p_run_euler_timestep_update,	///< pointer to function to compute euler time step updates
				prog_h, prog_u, prog_v,
				dt,
				simVars.timecontrol.current_timestep_size,
				simVars.disc.timestepping_runge_kutta_order,
				simVars.timecontrol.current_simulation_time
			);

		// provide information to parameters
		simVars.timecontrol.current_timestep_size = dt;
		simVars.timecontrol.current_simulation_time += dt;
		simVars.timecontrol.current_timestep_nr++;

#if SWEET_GUI
		timestep_output();
#endif
	}



public:
	void timestep_output(
			std::ostream &o_ostream = std::cout
	)
	{
		if (simVars.misc.verbosity > 0)
		{
			update_diagnostics();

			if (simVars.timecontrol.current_timestep_nr == 0)
			{
				o_ostream << "T\tMASS\tENERGY\tPOT_ENSTROPHY";

				if (simVars.setup.scenario == 2 || simVars.setup.scenario == 3 || simVars.setup.scenario == 4)
					o_ostream << "\tDIFF_P\tDIFF_U\tDIFF_V";

				o_ostream << std::endl;
			}

			o_ostream << simVars.timecontrol.current_simulation_time << "\t" << simVars.diag.total_mass << "\t" << simVars.diag.total_energy << "\t" << simVars.diag.total_potential_enstrophy;

			if (simVars.setup.scenario == 2 || simVars.setup.scenario == 3 || simVars.setup.scenario == 4)
			{
				for (std::size_t j = 0; j < simVars.disc.res[1]; j++)
					for (std::size_t i = 0; i < simVars.disc.res[0]; i++)
					{
						// h
						double x = (((double)i+0.5)/(double)simVars.disc.res[0])*simVars.sim.domain_size[0];
						double y = (((double)j+0.5)/(double)simVars.disc.res[1])*simVars.sim.domain_size[1];

						tmp.set(j, i, SWEValidationBenchmarks::return_h(simVars, x, y));
					}

				benchmark_diff_h = (prog_h-tmp).reduce_norm1_quad() / (double)(simVars.disc.res[0]*simVars.disc.res[1]);
				o_ostream << "\t" << benchmark_diff_h;

				// set data to something to overcome assertion error
				for (std::size_t j = 0; j < simVars.disc.res[1]; j++)
					for (std::size_t i = 0; i < simVars.disc.res[0]; i++)
					{
						// u space
						double x = (((double)i+0.5)/(double)simVars.disc.res[0])*simVars.sim.domain_size[0];
						double y = (((double)j+0.5)/(double)simVars.disc.res[1])*simVars.sim.domain_size[1];

						tmp.set(j, i, SWEValidationBenchmarks::return_u(simVars, x, y));
					}

				benchmark_diff_u = (prog_u-tmp).reduce_norm1_quad() / (double)(simVars.disc.res[0]*simVars.disc.res[1]);
				o_ostream << "\t" << benchmark_diff_u;

				for (std::size_t j = 0; j < simVars.disc.res[1]; j++)
					for (std::size_t i = 0; i < simVars.disc.res[0]; i++)
					{
						// v space
						double x = (((double)i+0.5)/(double)simVars.disc.res[0])*simVars.sim.domain_size[0];
						double y = (((double)j+0.5)/(double)simVars.disc.res[1])*simVars.sim.domain_size[1];

						tmp.set(j, i, SWEValidationBenchmarks::return_v(simVars, x, y));
					}

				benchmark_diff_v = (prog_v-tmp).reduce_norm1_quad() / (double)(simVars.disc.res[0]*simVars.disc.res[1]);
				o_ostream << "\t" << benchmark_diff_v;
			}

			o_ostream << std::endl;
		}
	}



public:
	bool should_quit()
	{
		if (simVars.timecontrol.max_timesteps_nr != -1 && simVars.timecontrol.max_timesteps_nr <= simVars.timecontrol.current_timestep_nr)
			return true;

		if (simVars.timecontrol.max_simulation_time != -1 && simVars.timecontrol.max_simulation_time <= simVars.timecontrol.current_simulation_time)
			return true;

		return false;
	}


	/**
	 * postprocessing of frame: do time stepping
	 */
	void vis_post_frame_processing(
			int i_num_iterations
	)
	{
		if (simVars.timecontrol.run_simulation_timesteps)
			for (int i = 0; i < i_num_iterations; i++)
				run_timestep();
	}


	struct VisStuff
	{
		const DataArray<2>* data;
		const char *description;
	};

	VisStuff vis_arrays[4] =
	{
			{&prog_h,	"h"},
			{&prog_u,	"u"},
			{&prog_v,	"v"},
			{&eta,		"eta"}
	};



	void vis_get_vis_data_array(
			const DataArray<2> **o_dataArray,
			double *o_aspect_ratio
	)
	{
		int id = simVars.misc.vis_id % (sizeof(vis_arrays)/sizeof(*vis_arrays));
		*o_dataArray = vis_arrays[id].data;
		*o_aspect_ratio = simVars.sim.domain_size[1] / simVars.sim.domain_size[0];
#if 0
		DataArray<2> &o = (DataArray<2> &)*vis_arrays[id].data;
		o.set_spec_all(0, 0);
		o.spec_set(0, 0, 1, 0);

		if (id == 0)
		{
			o.spec_set(0, 0, 1, 0);
		}
		else if (id == 1)
		{
			o.spec_set(0, 0, 1, 1);
		}
		else if (id == 2)
		{
			o.spec_set(0, 1, 1, 0);
		}
		else if (id == 3)
		{
			o.spec_set(0, 1, 1, 1);
//			o.spec_set(0, o.resolution_spec[0]-1, 1, 0);
		}
#endif
	}



	/**
	 * return status string for window title
	 */
	const char* vis_get_status_string()
	{
		// first, update diagnostic values if required
		update_diagnostics();

		int id = simVars.misc.vis_id % (sizeof(vis_arrays)/sizeof(*vis_arrays));

		static char title_string[2048];
		sprintf(title_string, "Time: %f (%.2f d), Timestep: %i, timestep size: %.14e, Vis: %.14s, Mass: %.14e, Energy: %.14e, Potential Entrophy: %.14e",
				simVars.timecontrol.current_simulation_time,
				simVars.timecontrol.current_simulation_time/(60.0*60.0*24.0),
				simVars.timecontrol.current_timestep_nr,
				simVars.timecontrol.current_timestep_size,
				vis_arrays[id].description,
				simVars.diag.total_mass, simVars.diag.total_energy, simVars.diag.total_potential_enstrophy);

		return title_string;
	}



	void vis_pause()
	{
		simVars.timecontrol.run_simulation_timesteps = !simVars.timecontrol.run_simulation_timesteps;
	}



	void vis_keypress(int i_key)
	{
		switch(i_key)
		{
		case 'v':
			simVars.misc.vis_id++;
			break;

		case 'V':
			simVars.misc.vis_id--;
			break;
		}
	}


	bool instability_detected()
	{
		return !(	prog_h.reduce_all_finite() &&
					prog_u.reduce_all_finite() &&
					prog_v.reduce_all_finite()
				);
	}
};



void compute_polvani_initialization(
		DataArray<2> &h,
		DataArray<2> &u,
		DataArray<2> &v
)
{

	{
		double m = 25;
		double k0 = 14;

		double R = 0.01;
		double F = 0.04;

		if (!std::isinf(simVars.bogus.var[0]))
			R = simVars.bogus.var[0];

		if (!std::isinf(simVars.bogus.var[1]))
			F = simVars.bogus.var[1];

		std::cout << "Using R of " << R << std::endl;
		std::cout << "Using F of " << F << std::endl;

		double B = (R*R)/(F*F);

		// initialize seed
	//	srand(0x15051982);
		srand(time(NULL));

		Operators2D op(simVars.disc.res, simVars.sim.domain_size, simVars.disc.use_spectral_basis_diffs);

		/*
		 * see Polvani et. al: Coherent structures of shallow-water turbulence
		 */

		/**
		 * Helpers
		 */
		DataArray<2> laplace_op = op.diff2_c_x+op.diff2_c_y;

		/**
		 * STEP 1) Initialize PSI
		 */

		DataArray<2> energy_init(simVars.disc.res);
		energy_init.set_spec_all(0,0);

		for (std::size_t j = 0; j < energy_init.resolution_spec[1]/2; j++)
		{
			for (std::size_t i = 0; i < energy_init.resolution_spec[0]/2; i++)
			{
				double k = std::sqrt((double)(i*i)+(double)(j*j));

				// compute energy spectrum
				double energy = pow(k, m*0.5)/pow(k+k0, m);

				assert(energy >= 0);
				energy_init.set_spec_spectrum(j, i, energy, 0);

/*
				if (ka*ka+kb*kb == 1)
					energy_init.spec_set(j, i, 1, 0);
				else
					energy_init.spec_set(j, i, 0, 0);*/

				/**
				 * Information on real FFT
				 * x and y are coordinates in the spectral space
				 * with x halfed in one dimension.
				 *
				 * x-direction:
				 *    real: cos frequencies of M_PI*2.0*x
				 *    complex: sin frequencies of M_PI*2.0*x
				 *
				 *    values at x=res-1: highest frequency
				 *
				 *    note, that the amplitudes for the frequencies set
				 *    for the x direction are of double magnitude compared
				 *    to each one in the y-direction. This is due to the
				 *    mirroring: Each value a-kind-of counts twice.
				 *
				 * y-direction:
				 *    real: cos frequencies of M_PI*2.0*y
				 *    complex: cos frequencies of M_PI*2.0*y
				 *
				 *    values at y=res-1: lowest non-constant frequency,
				 *    equal to freq at y=1
				 */
#if 0
				if (0)
				{
					if (j > 0)
					{
						energy_init.spec_set(j, i, 0, 0);
					}
					else
					{
						if (i == 0)
							energy_init.spec_set(j, i, 0, 0);
						else if (i == 1)
							energy_init.spec_set(j, i, 1, 0);
//						else if (i == energy_init.resolution_spec[0]-1)
//							energy_init.spec_set(j, i, 1, 0);
						else
							energy_init.spec_set(j, i, 0, 0);
					}
				}

				if (1)
				{
					if (i > 0)
					{
						energy_init.spec_set(j, i, 0, 0);
					}
					else
					{
						if (j == 0)
							energy_init.spec_set(j, i, 0, 0);
						else if (j == 1)
							energy_init.spec_set(j, i, 1, 0);
						else if (j == energy_init.resolution_spec[1]-1)
							energy_init.spec_set(j, i, 1, 0);
						else
							energy_init.spec_set(j, i, 0, 0);
					}
				}
#endif
			}
		}

		double scale_energyx = 0.5/energy_init.reduce_rms();
//		scale_energyx *= 0.5*sqrt(2.0);
		energy_init *= scale_energyx;

#if 0
		h = energy_init;
		u.set_all(0);
		v.set_all(0);
		return;
#endif

		/**
		 * See paper:
		 * The energy should be set, so that the rms speed of the initial velocity is 1:
		 *
		 * non-dimensional RMS speed = 1
		 *
		 * => rms speed = sqrt(1/N  \sum( sqrt(u^2+v^2) )) = 1
		 *
		 * rms(u) = rms(v) = 1/sqrt(2)
		 *
		 * => energy: |E| = 0.5*sqrt(1/N * 1)
		 */

		// compute rms
		double scale_energy = 1.0/energy_init.reduce_rms();

		// normalize to get desired velocity
		scale_energy *= 0.5;

		energy_init *= scale_energy;

		std::cout << "ENERGY rms: " << energy_init.reduce_rms() << std::endl;

		DataArray<2> psi(simVars.disc.res);
		psi.set_spec_all(0, 0);

		for (std::size_t j = 0; j < psi.resolution_spec[1]/2; j++)
		{
			for (std::size_t i = 0; i < psi.resolution_spec[0]/2; i++)
			{
				if (i == 0 && j == 0)
					continue;

				// compute energy spectrum
				double energy = energy_init.spec_getRe(j, i);

				double k = std::sqrt((double)(i*i)+(double)(j*j));
				double psi_abs = std::sqrt(energy*2.0/(k*k));

				{
					// compute random number \in [0;1[
					double r = (double)rand()/((double)RAND_MAX+1);

					// generate random phase shift
					double psi_re = cos(2.0*M_PIl*r);
					double psi_im = sin(2.0*M_PIl*r);

					psi_re *= psi_abs;
					psi_im *= psi_abs;

					psi.set_spec_spectrum_A(j, i, psi_re, psi_im);
				}
				{
					// compute random number \in [0;1[
					double r = (double)rand()/((double)RAND_MAX+1);

					// generate random phase shift
					double psi_re = cos(2.0*M_PIl*r);
					double psi_im = sin(2.0*M_PIl*r);

					psi_re *= psi_abs;
					psi_im *= psi_abs;

					psi.set_spec_spectrum_B(j, i, psi_re, psi_im);
				}
			}
		}

#if 0
		h = psi;
		u.set_all(0);
		v.set_all(0);
		return;
#endif

		std::cout << "CART Max: " << psi.reduce_maxAbs() << std::endl;
		std::cout << "SPEC Max: " << psi.reduce_spec_maxAbs() << std::endl;
		std::cout << "SPEC Centroid: " << psi.reduce_spec_getPolvaniCentroid() << std::endl;

		/**
		 * STEP 2) Compute h
		 */
		// equation 2.5b
		h = op.laplace(psi)+(2.0*R)*op.arakawa_jacobian(op.diff_c_x(psi), op.diff_c_y(psi));
		// (D*D)^-1
		h = h.spec_div_element_wise(laplace_op);

		// Add 1.0 for non-dimensional average height (not sure, if this is correct)
//		h += 1.0;

		/**
		 * STEP 3) Solve with iterations for \xi
		 */
		DataArray<2> xi(simVars.disc.res);
		xi.set_spec_all(0, 0);

		DataArray<2> prev_xi(simVars.disc.res);
		prev_xi.set_spec_all(0, 0);

		double prev_inf_norm = -1;
		double eps = 1e-9;


		int i = 0;
		for (; i < 100; i++)
		{
			// cache this value
			DataArray<2> laplace_psi = op.laplace(psi);

			/**
			 * we first solve psi_t for equation (2.5a)
			 */

			DataArray<2> psi_t =
				(
					op.arakawa_jacobian(psi, laplace_psi)
					+ (1.0/R)*op.laplace(xi)
					+ op.diff_c_x(laplace_psi*op.diff_c_x(xi))
					+ op.diff_c_y(laplace_psi*op.diff_c_y(xi))
				).spec_div_element_wise(laplace_op);


			////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////
			DataArray<2> psi_t_rhs =
				(
					op.arakawa_jacobian(psi, laplace_psi)
					+ (1.0/R)*op.laplace(xi)
					+ op.diff_c_x(laplace_psi*op.diff_c_x(xi))
					+ op.diff_c_y(laplace_psi*op.diff_c_y(xi))
				);

			DataArray<2> psi_t_lhs = op.laplace(psi_t);

			double check = (psi_t_lhs-psi_t_rhs).reduce_maxAbs();
			std::cout << check << std::endl;
			////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////

			/**
			 * Solve iterative equation (after equation 2.6)
			 */
			DataArray<2> rhs(simVars.disc.res);
			rhs.set_spec_all(0, 0);

			// RHS, 1st line
			rhs += -op.arakawa_jacobian(psi, op.laplace(xi));
			rhs += op.laplace(op.arakawa_jacobian(psi, h));

			// RHS, 2nd line
			// special jacobian, see Footnote 2 on page 186
			DataArray<2> spec_jacob =
					op.diff2_c_x(psi_t)*op.diff2_c_y(psi)
					+ op.diff2_c_x(psi)*op.diff2_c_y(psi_t)
					- 2.0*op.diff_c_x(op.diff_c_y(psi))*op.diff_c_x(op.diff_c_y(psi_t))
				;
			rhs += 2.0*R*spec_jacob;
			rhs += -op.diff_c_x(laplace_psi*op.diff_c_x(xi)) - op.diff_c_y(laplace_psi*op.diff_c_y(xi));

			// RHS, 3rd line
			rhs += op.laplace(
					op.diff_c_x(h*op.diff_c_x(xi)) +
					op.diff_c_y(h*op.diff_c_y(xi))
				);

			// LHS, 1st line
			DataArray<2> lhs(simVars.disc.res);
			lhs.set_spec_all(0, 0);

			// IMPORTANT! Apply last operator element-wise
			lhs += (1.0/R)*(laplace_op-B*laplace_op*laplace_op);

			xi = rhs.spec_div_element_wise(lhs);

			double new_inf_norm;
			// convergence check
			if (0)
			{
				// compute difference on both sides of equations in xi

				DataArray<2> psi_t =
					(
						op.arakawa_jacobian(psi, laplace_psi)
						+ (1.0/R)*op.laplace(xi)
						+ op.diff_c_x(laplace_psi*op.diff_c_x(xi))
						+ op.diff_c_y(laplace_psi*op.diff_c_y(xi))
					).spec_div_element_wise(laplace_op);

				DataArray<2> spec_jacob =
						op.diff2_c_x(psi_t)*op.diff2_c_y(psi)
						+ op.diff2_c_x(psi)*op.diff2_c_y(psi_t)
						- 2.0*op.diff_c_x(op.diff_c_y(psi))*op.diff_c_x(op.diff_c_y(psi_t))
					;

				new_inf_norm =
						(
							(1.0/R)*(op.laplace(xi)-B*op.laplace(op.laplace(xi)))	/// LHS
							-
							(-1.0)*op.arakawa_jacobian(psi, op.laplace(xi))
							+ op.laplace(op.arakawa_jacobian(psi, h))
							+ 2.0*R*op.arakawa_jacobian(op.diff_c_x(psi), op.diff_c_y(psi))
								- op.diff_c_x(op.laplace(psi)*op.diff_c_x(xi))
								- op.diff_c_y(op.laplace(psi)*op.diff_c_y(xi))
							+ op.laplace(
									op.diff_c_x(h*op.diff2_c_x(xi))
									+ op.diff_c_y(h*op.diff2_c_y(xi))
								)
						).reduce_maxAbs();//*normalize_norm;
			}
			else
			{
				new_inf_norm = (prev_xi-xi).reduce_maxAbs();
			}

			double normalization = xi.reduce_norm1()/(double)(simVars.disc.res[0]*simVars.disc.res[1]);

			double convergence = std::abs(prev_inf_norm - new_inf_norm)/normalization;
			std::cout << "INF norm / convergence: " << new_inf_norm << " / " << convergence << std::endl;

			if (convergence < eps)
			{
				std::cout << "CONVERGED with norm: " << new_inf_norm << std::endl;
				break;
			}
			prev_inf_norm = new_inf_norm;
			prev_xi = xi;
		};

		if (i == 100)
		{
			std::cerr << "NO CONVERGENCE!" << std::endl;
			exit(1);
		}

		double epsilon = R * (std::max(1.0,R)/std::max(1.0,B));

		u = op.diff_c_y(psi) + epsilon*op.diff_c_x(xi);
		v = -op.diff_c_x(psi) + epsilon*op.diff_c_y(xi);

		double rms_u = u.reduce_rms();
		u /= rms_u*sqrt(2.0);
	//	rms_u = u.reduce_rms();

		double rms_v = v.reduce_rms();
		v /= rms_v*sqrt(2.0);
	//	rms_v = v.reduce_rms();

		std::cout << "Target RMS u, v: " << 1.0/sqrt(2.0) << ", " << 1.0/sqrt(2.0) << std::endl;
		std::cout << "Current RMS u, v: " << rms_u << ", " << rms_v << std::endl;

		/**
		 * Check results
		 */
		{
//			double normalize_norm = 1.0/(double)(parameters.discretization.res[0]*parameters.discretization.res[1]);

			DataArray<2> laplace_psi = op.laplace(psi);
			double psi_l1_norm =
					(	op.laplace(psi)
						-
						op.arakawa_jacobian(psi, op.laplace(psi))
						+(1.0/R)*op.laplace(xi)
						+op.diff_c_x(op.laplace(psi)*op.diff_c_x(xi))
						+op.diff_c_y(op.laplace(psi)*op.diff_c_y(xi))
					).reduce_maxAbs();//*normalize_norm;
			std::cout << "PSI l1 norm: " << psi_l1_norm << std::endl;

			double h_l1_norm =
					(
						op.laplace(h)
						-
						op.laplace(psi)
						+2.0*R*op.arakawa_jacobian(op.diff_c_x(psi), op.diff_c_y(psi))
					).reduce_maxAbs();//*normalize_norm;
			std::cout << "H l1 norm: " << h_l1_norm << std::endl;


			DataArray<2> psi_t =
				(
					op.arakawa_jacobian(psi, laplace_psi)
					+ (1.0/R)*op.laplace(xi)
					+ op.diff_c_x(laplace_psi*op.diff_c_x(xi))
					+ op.diff_c_y(laplace_psi*op.diff_c_y(xi))
				).spec_div_element_wise(laplace_op);

			DataArray<2> spec_jacob =
					op.diff2_c_x(psi_t)*op.diff2_c_y(psi)
					+ op.diff2_c_x(psi)*op.diff2_c_y(psi_t)
					- 2.0*op.diff_c_x(op.diff_c_y(psi))*op.diff_c_x(op.diff_c_y(psi_t))
				;

			double xi_l1_norm =
					(
						(1.0/R)*(op.laplace(xi)-B*op.laplace(op.laplace(xi)))	/// LHS
						-
						(-1.0)*op.arakawa_jacobian(psi, op.laplace(xi))
						+ op.laplace(op.arakawa_jacobian(psi, h))
						+ 2.0*R*op.arakawa_jacobian(op.diff_c_x(psi), op.diff_c_y(psi))
							- op.diff_c_x(op.laplace(psi)*op.diff_c_x(xi))
							- op.diff_c_y(op.laplace(psi)*op.diff_c_y(xi))
						+ op.laplace(
								op.diff_c_x(h*op.diff2_c_x(xi))
								+ op.diff_c_y(h*op.diff2_c_y(xi))
							)
					).reduce_maxAbs();//*normalize_norm;
			std::cout << "xi l1 norm: " << xi_l1_norm << std::endl;
		}

		DataArray<2> energy = 0.5*(u*u+v*v);
		std::cout << "ENERGY Centroid: " << energy.reduce_spec_getPolvaniCentroid() << std::endl;
		std::cout << std::endl;
		std::cout << "R: " << R << std::endl;
		std::cout << "F: " << F << std::endl;
		std::cout << "B: " << B << std::endl;
		std::cout << "XIrms/PSIrms: " << xi.reduce_rms()/psi.reduce_rms() << std::endl;
	}

}


int main(int i_argc, char *i_argv[])
{

	const char *bogus_var_names[] = {
			"polvani-R",
			"polvani-F",
			nullptr
	};


//	simVars.bogus.var[0] = 0.0;	// R
//	simVars.bogus.var[1] = 0.0;	// F

	if (!simVars.setupFromMainParameters(i_argc, i_argv, bogus_var_names))
	{
		std::cout << "	--polvani-R [param]" << std::endl;
		std::cout << "	--polvani-F [param]" << std::endl;
		return -1;
	}

	DataArray<2> local_h(simVars.disc.res);
	DataArray<2> local_u(simVars.disc.res);
	DataArray<2> local_v(simVars.disc.res);

	if (!std::isinf(simVars.bogus.var[0]))
	{
		if (simVars.sim.domain_size[0] != 1.0)
		{
			std::cout << "Domain length should be set to 1 for non-dimensional Polvani test case" << std::endl;
			return 1;
		}

		if (simVars.sim.domain_size[1] != 1.0)
		{
			std::cout << "Domain length should be set to 1 for non-dimensional Polvani test case" << std::endl;
			return 1;
		}

		compute_polvani_initialization(local_h, local_u, local_v);

		::init_h = &local_h;
		::init_u = &local_u;
		::init_v = &local_v;
	}


	if (1)
	{
		SimulationSWE *simulationSWE = new SimulationSWE;

		std::ostringstream buf;
		buf << std::setprecision(14);

	#if SWEET_GUI
		VisSweet<SimulationSWE> visSweet(simulationSWE);
	#else
		simulationSWE->reset();

		Stopwatch time;
		time.reset();

		double diagnostics_energy_start, diagnostics_mass_start, diagnostics_potential_entrophy_start;

		if (simVars.misc.verbosity > 1)
		{
			simulationSWE->update_diagnostics();
			diagnostics_energy_start = simVars.diag.total_energy;
			diagnostics_mass_start = simVars.diag.total_mass;
			diagnostics_potential_entrophy_start = simVars.diag.total_potential_enstrophy;
		}

		while(true)
		{
			if (simVars.misc.verbosity > 1)
			{
				simulationSWE->timestep_output(buf);

				std::string output = buf.str();
				buf.str("");

				std::cout << output;

				if (simVars.misc.verbosity > 2)
					std::cerr << output;
			}

			if (simulationSWE->should_quit())
				break;

			simulationSWE->run_timestep();

			if (simulationSWE->instability_detected())
			{
				std::cout << "INSTABILITY DETECTED" << std::endl;
				break;
			}
		}

		time.stop();

		double seconds = time();

		std::cout << "Simulation time: " << seconds << " seconds" << std::endl;
		std::cout << "Time per time step: " << seconds/(double)simVars.timecontrol.current_timestep_nr << " sec/ts" << std::endl;



		if (simVars.misc.verbosity > 1)
		{
			std::cout << "DIAGNOSTICS ENERGY DIFF:\t" << std::abs((simVars.diag.total_energy-diagnostics_energy_start)/diagnostics_energy_start) << std::endl;
			std::cout << "DIAGNOSTICS MASS DIFF:\t" << std::abs((simVars.diag.total_mass-diagnostics_mass_start)/diagnostics_mass_start) << std::endl;
			std::cout << "DIAGNOSTICS POTENTIAL ENSTROPHY DIFF:\t" << std::abs((simVars.diag.total_potential_enstrophy-diagnostics_potential_entrophy_start)/diagnostics_potential_entrophy_start) << std::endl;

			if (simVars.setup.scenario == 2 || simVars.setup.scenario == 3 || simVars.setup.scenario == 4)
			{
				std::cout << "DIAGNOSTICS BENCHMARK DIFF H:\t" << simulationSWE->benchmark_diff_h << std::endl;
				std::cout << "DIAGNOSTICS BENCHMARK DIFF U:\t" << simulationSWE->benchmark_diff_u << std::endl;
				std::cout << "DIAGNOSTICS BENCHMARK DIFF V:\t" << simulationSWE->benchmark_diff_v << std::endl;
			}
		}
	#endif

		delete simulationSWE;
	}

	return 0;
}
