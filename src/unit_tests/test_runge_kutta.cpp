
#include <sweet/DataArray.hpp>
#if SWEET_GUI
	#include <sweet/VisSweet.hpp>
#endif
#include <sweet/SimulationParameters.hpp>
#include <sweet/TimesteppingRK.hpp>
#include <sweet/SWEValidationBenchmarks.hpp>
#include <sweet/Operators2D.hpp>
#include <sweet/Stopwatch.hpp>

#include <ostream>
#include <sstream>
#include <unistd.h>
#include <iomanip>
#include <stdio.h>



SimulationParameters parameters;

int time_test_function_order = 0;
int timestepping_runge_kutta_order = 0;

class SimulationTestRK
{
public:
	DataArray<2> prog_h;
	DataArray<2> prog_u;
	DataArray<2> prog_v;

	Operators2D op;

	TimesteppingRK timestepping;


	/**
	 * Function of 4th order.
	 */
public:
	double test_function(int i_order, double z)
	{
		switch(i_order)
		{
		case 0:
			return	2.0;

		case 1:
			return	z
					+2.0;

		case 2:
			return	+(5./12.)*z*z
					+z
					+2.0;

		case 3:
			return	-(1./2.)*z*z*z
					+(5./12.)*z*z
					+z
					+2.0;

		case 4:
			return	(1./12.)*z*z*z*z
					-(1./2.)*z*z*z
					+(5./12.)*z*z
					+z
					+2.0;
		}
	}



public:
	double test_function_diff(int i_order, double z)
	{
		switch(i_order)
		{
		case 0:
			return	0.0;

		case 1:
			return	1.0;

		case 2:
			return	+(10./12.)*z
					+1.0;

		case 3:
			return	-(3./2.)*z*z
					+(10./12.)*z
					+1.0;

		case 4:
			return	(4./12.)*z*z*z
					-(3./2.)*z*z
					+(10./12.)*z
					+1.0;
		}
	}


public:
	SimulationTestRK()	:
		prog_h(parameters.res),
		prog_u(parameters.res),
		prog_v(parameters.res),

		op(parameters.res, parameters.sim_domain_size, parameters.use_spectral_diffs)
	{
		reset();
	}


	void reset()
	{
		parameters.status_timestep_nr = 0;

		prog_h.setAll(test_function(time_test_function_order, 0));
		prog_u.setAll(0);
		prog_v.setAll(0);
	}


	void p_run_euler_timestep_update(
			const DataArray<2> &i_h,	///< prognostic variables
			const DataArray<2> &i_u,	///< prognostic variables
			const DataArray<2> &i_v,	///< prognostic variables

			DataArray<2> &o_h_t,	///< time updates
			DataArray<2> &o_u_t,	///< time updates
			DataArray<2> &o_v_t,	///< time updates

			double &o_dt,			///< time step restriction
			double i_fixed_dt = 0,	///< if this value is not equal to 0, use this time step size instead of computing one
			double i_current_timestamp = -1
	)
	{
		o_h_t.setAll(test_function_diff(time_test_function_order, i_current_timestamp));
		o_u_t.setAll(0);
		o_v_t.setAll(0);

		if (parameters.sim_CFL < 0)
			o_dt = -parameters.sim_CFL;
		else
			o_dt = 0.1;

		parameters.status_timestep_nr++;
	}



	void run()
	{
	}



	void run_timestep()
	{
		double dt;

		timestepping.run_rk_timestep(
				this,
				&SimulationTestRK::p_run_euler_timestep_update,	///< pointer to function to compute euler time step updates
				prog_h, prog_u, prog_v,
				dt,
				parameters.timestepping_timestep_size,
				timestepping_runge_kutta_order,
				parameters.status_simulation_time
			);

		// provide information to parameters
		parameters.status_simulation_timestep_size = dt;
		parameters.status_simulation_time += dt;
		parameters.status_timestep_nr++;
	}



	bool should_quit()
	{
		return false;
	}



	/**
	 * postprocessing of frame: do time stepping
	 */
	void vis_post_frame_processing(int i_num_iterations)
	{
		if (parameters.run_simulation)
			for (int i = 0; i < i_num_iterations; i++)
				run_timestep();
	}



	void vis_get_vis_data_array(
			const DataArray<2> **o_dataArray,
			double *o_aspect_ratio
	)
	{
		*o_dataArray = &prog_h;
		*o_aspect_ratio = parameters.sim_domain_size[1] / parameters.sim_domain_size[0];
	}

	const char* vis_get_status_string()
	{
		static char title_string[1024];
		sprintf(title_string, "Time (days): %f (%.2f d), Timestep: %i, timestep size: %.14e, Mass: %.14e, Energy: %.14e, Potential Entrophy: %.14e",
				parameters.status_simulation_time,
				parameters.status_simulation_time/(60.0*60.0*24.0),
				parameters.status_timestep_nr,
				parameters.status_simulation_timestep_size,
				parameters.diagnostics_mass, parameters.diagnostics_energy, parameters.diagnostics_potential_entrophy);
		return title_string;
	}

	void vis_pause()
	{
		parameters.run_simulation = !parameters.run_simulation;
	}


	void vis_keypress(int i_key)
	{
		switch(i_key)
		{
		case 'v':
			parameters.vis_id++;
			break;

		case 'V':
			parameters.vis_id--;
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



int main(
		int i_argc,
		char *i_argv[]
)
{
	parameters.setup(i_argc, i_argv);

#if SWEET_GUI
	if (parameters.gui_enabled)
	{
		SimulationTestRK *simulationTestRK = new SimulationTestRK;
		VisSweet<SimulationTestRK> visSweet(simulationTestRK);
		delete simulationTestRK;
		return 0;
	}
#endif

	if (parameters.max_simulation_time == -1)
	{
		std::cerr << "Max. simulation time is unlimited, please specify e.g. -t 6" << std::endl;
		return -1;
	}

	if (!std::isinf(parameters.bogus_var0))
		time_test_function_order = parameters.bogus_var0;

	for (int fun_order = 0; fun_order <= 4; fun_order++)
	{
		time_test_function_order = fun_order;

		for (int ts_order = 1; ts_order <= 4; ts_order++)
		{
			timestepping_runge_kutta_order = ts_order;

			/*
			 * iterate over resolutions, starting by res[0] given e.g. by program parameter -n
			 */
			parameters.reset();

			SimulationTestRK *simulationTestRK = new SimulationTestRK;

			while(true)
			{
				if (parameters.verbosity > 2)
					std::cout << parameters.status_simulation_time << ": " << simulationTestRK->prog_h.get(0,0) << std::endl;

				simulationTestRK->run_timestep();

				if (simulationTestRK->instability_detected())
				{
					std::cout << "INSTABILITY DETECTED" << std::endl;
					break;
				}

				if (parameters.max_simulation_time < parameters.status_simulation_time)
				{
					DataArray<2> benchmark_h(parameters.res);

					benchmark_h.setAll(simulationTestRK->test_function(time_test_function_order, parameters.status_simulation_time));

					double error = (simulationTestRK->prog_h-benchmark_h).reduce_rms_quad();
					std::cout << "with function order " << fun_order << " with RK timestepping " << ts_order << " resulted in RMS error " << error << std::endl;

					if (fun_order <= ts_order)
					{
						if (error > 0.0000001)
						{
							std::cout << "ERROR threshold exceeded!" << std::endl;
							return 1;
						}
					}
					else
					{
						if (error < 0.0000001)
						{
							std::cout << "ERROR threshold is expected to be larger, can still be valid!" << std::endl;
							return 1;
						}
					}
					std::cout << "OK" << std::endl;

					break;
				}
			}

			delete simulationTestRK;
		}
	}


	return 0;
}
