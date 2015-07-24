
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


//
// 0: Gaussian (WARNING! DON'T USE THIS AS INITIAL CONDITIONS!)
// 1: sin curve
//
#define ADV_FUNCTION	1

class SimulationAdvection
{
public:
	DataArray<2> prog_h;
	DataArray<2> prog_u;
	DataArray<2> prog_v;

	DataArray<2> hu;
	DataArray<2> hv;

	DataArray<2> tmp;

	Operators2D op;

	TimesteppingRK timestepping;

#if ADV_FUNCTION==1
	double freq_x = 4.0;
	double freq_y = 4.0;
#endif


public:
	SimulationAdvection()	:
		prog_h(parameters.res),
		prog_u(parameters.res),
		prog_v(parameters.res),

		hu(parameters.res),
		hv(parameters.res),

		tmp(parameters.res),

		op(parameters.res, parameters.sim_domain_size, parameters.use_spectral_diffs)
	{
		reset();
	}


	DataArray<2> get_advected_solution(
		double i_timestamp
	)
	{
		DataArray<2> ret_h(prog_h.resolution);

		double adv_x = (std::isinf(parameters.bogus_var0) ? 0 : -parameters.bogus_var0*i_timestamp);
		double adv_y = (std::isinf(parameters.bogus_var1) ? 0 : -parameters.bogus_var1*i_timestamp);

		double radius = parameters.setup_radius_scale*
			std::sqrt(
				 (double)parameters.sim_domain_size[0]*(double)parameters.sim_domain_size[0]
				+(double)parameters.sim_domain_size[1]*(double)parameters.sim_domain_size[1]
			);

		for (std::size_t j = 0; j < parameters.res[1]; j++)
		{
			for (std::size_t i = 0; i < parameters.res[0]; i++)
			{
#if ADV_FUNCTION==0

				double x = (((double)i+0.5)/(double)parameters.res[0])*parameters.sim_domain_size[0];
				double y = (((double)j+0.5)/(double)parameters.res[1])*parameters.sim_domain_size[1];

				x += adv_x;
				y += adv_y;

				if (x < 0)	x = parameters.sim_domain_size[0]-std::fmod(-x, parameters.sim_domain_size[0]);
				else		x = std::fmod(x, parameters.sim_domain_size[0]);

				if (y < 0)	y = parameters.sim_domain_size[1]-std::fmod(-y, parameters.sim_domain_size[1]);
				else		y = std::fmod(y, parameters.sim_domain_size[1]);

				double dx = x-parameters.setup_coord_x*parameters.sim_domain_size[0];
				double dy = y-parameters.setup_coord_y*parameters.sim_domain_size[1];

				dx /= radius;
				dy /= radius;

				double value = parameters.setup_h0+std::exp(-50.0*(dx*dx + dy*dy));
				ret_h.set(j, i, value);

#elif ADV_FUNCTION==1
				double x = (((double)i+0.5)/(double)parameters.res[0])*parameters.sim_domain_size[0];
				double y = (((double)j+0.5)/(double)parameters.res[1])*parameters.sim_domain_size[1];

				x += adv_x;
				y += adv_y;

				if (x < 0)	x = parameters.sim_domain_size[0]-std::fmod(-x, parameters.sim_domain_size[0]);
				else		x = std::fmod(x, parameters.sim_domain_size[0]);

				if (y < 0)	y = parameters.sim_domain_size[1]-std::fmod(-y, parameters.sim_domain_size[1]);
				else		y = std::fmod(y, parameters.sim_domain_size[1]);

				x /= parameters.sim_domain_size[0];
				y /= parameters.sim_domain_size[1];

				ret_h.set(j, i, std::sin(freq_x*M_PIl*x)*std::sin(freq_x*M_PIl*y));
#endif
			}
		}

		return ret_h;
	}



	DataArray<2> get_advected_solution_diffx(
		double i_timestamp
	)
	{
		DataArray<2> ret_h(prog_h.resolution);

		double adv_x = (std::isinf(parameters.bogus_var0) ? 0 : -parameters.bogus_var0*i_timestamp);
		double adv_y = (std::isinf(parameters.bogus_var1) ? 0 : -parameters.bogus_var1*i_timestamp);

		double radius_scale = std::sqrt(
				 (double)parameters.sim_domain_size[0]*(double)parameters.sim_domain_size[0]
				+(double)parameters.sim_domain_size[1]*(double)parameters.sim_domain_size[1]
			);

		double radius = parameters.setup_radius_scale*radius_scale;

		for (std::size_t j = 0; j < parameters.res[1]; j++)
		{
			for (std::size_t i = 0; i < parameters.res[0]; i++)
			{
#if ADV_FUNCTION==0

				double x = (((double)i+0.5)/(double)parameters.res[0])*parameters.sim_domain_size[0];
				double y = (((double)j+0.5)/(double)parameters.res[1])*parameters.sim_domain_size[1];

				x += adv_x;
				y += adv_y;

				if (x < 0)	x = parameters.sim_domain_size[0]-std::fmod(-x, parameters.sim_domain_size[0]);
				else		x = std::fmod(x, parameters.sim_domain_size[0]);

				if (y < 0)	y = parameters.sim_domain_size[1]-std::fmod(-y, parameters.sim_domain_size[1]);
				else		y = std::fmod(y, parameters.sim_domain_size[1]);

				double dx = x-parameters.setup_coord_x*parameters.sim_domain_size[0];
				double dy = y-parameters.setup_coord_y*parameters.sim_domain_size[1];

				dx /= radius;
				dy /= radius;

				double value = -50.0*2.0*dx*std::exp(-50.0*(dx*dx + dy*dy));
				value /= radius_scale;

				ret_h.set(j, i, value);

#elif ADV_FUNCTION==1

				double x = (((double)i+0.5)/(double)parameters.res[0])*parameters.sim_domain_size[0];
				double y = (((double)j+0.5)/(double)parameters.res[1])*parameters.sim_domain_size[1];

				x += adv_x;
				y += adv_y;

				if (x < 0)	x = parameters.sim_domain_size[0]-std::fmod(-x, parameters.sim_domain_size[0]);
				else		x = std::fmod(x, parameters.sim_domain_size[0]);

				if (y < 0)	y = parameters.sim_domain_size[1]-std::fmod(-y, parameters.sim_domain_size[1]);
				else		y = std::fmod(y, parameters.sim_domain_size[1]);

				x /= parameters.sim_domain_size[0];
				y /= parameters.sim_domain_size[1];

				ret_h.set(j, i, freq_x*M_PIl*std::cos(freq_x*M_PIl*x)*std::sin(freq_y*M_PIl*y)/parameters.sim_domain_size[0]);
#endif
			}
		}

		return ret_h;
	}



	DataArray<2> get_advected_solution_diffy(
		double i_timestamp
	)
	{
		DataArray<2> ret_h(prog_h.resolution);

		double adv_x = (std::isinf(parameters.bogus_var0) ? 0 : -parameters.bogus_var0*i_timestamp);
		double adv_y = (std::isinf(parameters.bogus_var1) ? 0 : -parameters.bogus_var1*i_timestamp);

		double radius_scale = std::sqrt(
				 (double)parameters.sim_domain_size[0]*(double)parameters.sim_domain_size[0]
				+(double)parameters.sim_domain_size[1]*(double)parameters.sim_domain_size[1]
			);

		double radius = parameters.setup_radius_scale*radius_scale;

		for (std::size_t j = 0; j < parameters.res[1]; j++)
		{
			for (std::size_t i = 0; i < parameters.res[0]; i++)
			{
#if ADV_FUNCTION==0

				double x = (((double)i+0.5)/(double)parameters.res[0])*parameters.sim_domain_size[0];
				double y = (((double)j+0.5)/(double)parameters.res[1])*parameters.sim_domain_size[1];

				x += adv_x;
				y += adv_y;

				if (x < 0)	x = parameters.sim_domain_size[0]-std::fmod(-x, parameters.sim_domain_size[0]);
				else		x = std::fmod(x, parameters.sim_domain_size[0]);

				if (y < 0)	y = parameters.sim_domain_size[1]-std::fmod(-y, parameters.sim_domain_size[1]);
				else		y = std::fmod(y, parameters.sim_domain_size[1]);

				double dx = x-parameters.setup_coord_x*parameters.sim_domain_size[0];
				double dy = y-parameters.setup_coord_y*parameters.sim_domain_size[1];

				dx /= radius;
				dy /= radius;

				double value = -50.0*2.0*dy*std::exp(-50.0*(dx*dx + dy*dy));
				value /= radius_scale;

				ret_h.set(j, i, value);

#elif ADV_FUNCTION==1

				double x = (((double)i+0.5)/(double)parameters.res[0])*parameters.sim_domain_size[0];
				double y = (((double)j+0.5)/(double)parameters.res[1])*parameters.sim_domain_size[1];

				x += adv_x;
				y += adv_y;

				if (x < 0)	x = parameters.sim_domain_size[0]-std::fmod(-x, parameters.sim_domain_size[0]);
				else		x = std::fmod(x, parameters.sim_domain_size[0]);

				if (y < 0)	y = parameters.sim_domain_size[1]-std::fmod(-y, parameters.sim_domain_size[1]);
				else		y = std::fmod(y, parameters.sim_domain_size[1]);

				x /= parameters.sim_domain_size[0];
				y /= parameters.sim_domain_size[1];

				ret_h.set(j, i, freq_y*M_PIl*std::sin(freq_x*M_PIl*x)*std::cos(freq_y*M_PIl*y)/parameters.sim_domain_size[1]);
#endif
			}
		}

		return ret_h;
	}



	void reset()
	{
		parameters.status_timestep_nr = 0;
		parameters.status_simulation_time = 0;
		parameters.status_simulation_timestep_size = -1;

		if (std::isinf(parameters.bogus_var0))
			prog_u.setAll(0);
		else
			prog_u.setAll(parameters.bogus_var0);

		if (std::isinf(parameters.bogus_var1))
			prog_v.setAll(0);
		else
			prog_v.setAll(parameters.bogus_var1);

		prog_h = get_advected_solution(0);
	}



	void p_run_euler_timestep_update(
			const DataArray<2> &i_h,	///< prognostic variables
			const DataArray<2> &i_u,	///< prognostic variables
			const DataArray<2> &i_v,	///< prognostic variables

			DataArray<2> &o_h_t,		///< time updates
			DataArray<2> &o_u_t,		///< time updates
			DataArray<2> &o_v_t,		///< time updates

			double &o_dt,				///< time step restriction
			double i_fixed_dt = 0,		///< if this value is not equal to 0, use this time step size instead of computing one
			double i_simulation_timestamp = -1
	)
	{
		if (parameters.bogus_var2 == 0)
		{
#if SWEET_USE_SPECTRAL_SPACE
			static bool output_given = false;
			if (!output_given)
			{
				std::cout << "WARNING: upwinding in spectral space not working" << std::endl;
				output_given = true;
			}
#endif

			o_h_t =
				(
					(
						// u is positive
						op.shift_right(i_h)*i_u.return_value_if_positive()	// inflow
						-i_h*op.shift_left(i_u.return_value_if_positive())	// outflow

						// u is negative
						+(i_h*i_u.return_value_if_negative())					// outflow
						-op.shift_left(i_h*i_u.return_value_if_negative())	// inflow
					)*(1.0/parameters.sim_cell_size[0])				// here we see a finite-difference-like formulation
					+
					(
						// v is positive
						op.shift_up(i_h)*i_v.return_value_if_positive()		// inflow
						-i_h*op.shift_down(i_v.return_value_if_positive())	// outflow

						// v is negative
						+(i_h*i_v.return_value_if_negative())					// outflow
						-op.shift_down(i_h*i_v.return_value_if_negative())	// inflow
					)*(1.0/parameters.sim_cell_size[1])
				);
		}
		else if (parameters.bogus_var2 == 1)
		{
			//             |                       |                       |
			// --v---------|-----------v-----------|-----------v-----------|
			//   h-1       u0          h0          u1          h1          u2

			// staggered
			o_h_t = -(
					op.diff_f_x(op.avg_b_x(i_h)*i_u) +
					op.diff_f_y(op.avg_b_y(i_h)*i_v)
				);
		}
		else  if (parameters.bogus_var2 == 2)
		{
			if (parameters.bogus_var3 == 0)
			{
				// non-staggered
				o_h_t = -(
						op.diff_c_x(i_h*i_u) +
						op.diff_c_y(i_h*i_v)
					);
			}
			else if (parameters.bogus_var3 == 1)
			{
				// non-staggered with analytical solution, only works for constant velocity!
				o_h_t = -(
						get_advected_solution_diffx(i_simulation_timestamp)*i_u +
						get_advected_solution_diffy(i_simulation_timestamp)*i_v
					);
			}
			else
			{
				std::cerr << "Usage of analytical solution not specified, use -d option [0: compute diffs on discrete solution, 1: use analytical diffs]" << std::endl;
				exit(-1);
			}
		}
		else  if (parameters.bogus_var2 == 3)
		{
			o_h_t.setAll(0);
		}
		else
		{
			std::cerr << "Advection type not specified, use -c option [0: up/downwinding, 1: staggered, 2: non-staggered]" << std::endl;
			exit(-1);
		}


		if (i_fixed_dt > 0)
		{
			o_dt = i_fixed_dt;
		}
		else
		{
			if (i_fixed_dt < 0)
			{
				o_dt = -i_fixed_dt;
			}
			else
			{
				if (parameters.sim_CFL < 0)
					o_dt = -parameters.sim_CFL*std::min(parameters.sim_cell_size[0]/i_u.reduce_maxAbs(), parameters.sim_cell_size[1]/i_v.reduce_maxAbs());
				else
					o_dt = parameters.sim_CFL*std::min(parameters.sim_cell_size[0]/i_u.reduce_maxAbs(), parameters.sim_cell_size[1]/i_v.reduce_maxAbs());
			}

		}

		o_u_t.setAll(0);
		o_v_t.setAll(0);

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
				&SimulationAdvection::p_run_euler_timestep_update,	///< pointer to function to compute euler time step updates
				prog_h, prog_u, prog_v,
				dt,
				parameters.timestepping_timestep_size,
				parameters.timestepping_runge_kutta_order,
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
		int vis_id = parameters.vis_id % 6;

		switch (vis_id)
		{
		default:
			*o_dataArray = &prog_h;
			break;

		case 1:
			tmp = get_advected_solution(parameters.status_simulation_time);
			*o_dataArray = &tmp;
			break;

		case 2:
			tmp = op.diff_c_x(get_advected_solution(parameters.status_simulation_time));
			*o_dataArray = &tmp;
			break;

		case 3:
			tmp = get_advected_solution_diffx(parameters.status_simulation_time);
			*o_dataArray = &tmp;
			break;

		case 4:
			tmp = op.diff_c_y(get_advected_solution(parameters.status_simulation_time));
			*o_dataArray = &tmp;
			break;

		case 5:
			tmp = get_advected_solution_diffy(parameters.status_simulation_time);
			*o_dataArray = &tmp;
			break;
		}

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
				parameters.diagnostics_mass,
				parameters.diagnostics_energy,
				parameters.diagnostics_potential_entrophy
			);

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


double compute_current_error(
		SimulationAdvection *simulationAdvection
)
{
	DataArray<2> benchmark_h = simulationAdvection->get_advected_solution(parameters.status_simulation_time);

	return (simulationAdvection->prog_h-benchmark_h).reduce_rms_quad();
}



int main(
		int i_argc,
		char *i_argv[]
)
{
	std::cout << std::setprecision(14);
	std::cerr << std::setprecision(14);

	parameters.setup(i_argc, i_argv);

	double u, v;
	if (std::isinf(parameters.bogus_var0))
		u = 0;
	else
		u = parameters.bogus_var0;

	if (std::isinf(parameters.bogus_var1))
		v = 0;
	else
		v = parameters.bogus_var1;

	double total_speed;
	double turnaround_time;
	if (u == 0 && v == 0)
	{
		std::cerr << "Both velocity components are zero, EXIT" << std::endl;
		exit(1);
	}

	if (u != 0 && v == 0)
	{
		total_speed = u;
		turnaround_time = parameters.sim_domain_size[0]/u;
	}
	else if (u == 0 && v != 0)
	{
		total_speed = v;
		turnaround_time = parameters.sim_domain_size[1]/v;
	}
	else
	{
		total_speed = v;
		if (std::abs(parameters.sim_domain_size[1]/parameters.sim_domain_size[0]-v/u) > 0.000000001)
		{
			std::cerr << "ratio of domain sizes and speed have to be similar" << std::endl;
			exit(1);
		}

		total_speed = std::sqrt(u*u+v*v);
		double diagonal = std::sqrt(parameters.sim_domain_size[0]*parameters.sim_domain_size[0] + parameters.sim_domain_size[1]*parameters.sim_domain_size[1]);
		turnaround_time = diagonal/total_speed;
	}

	if (parameters.verbosity > 1)
	{
		std::cout << "Turnaround time: " << turnaround_time << std::endl;
		std::cout << "Total speed: " << total_speed << std::endl;
	}

#if SWEET_GUI
	if (parameters.gui_enabled)
	{
		SimulationAdvection *simulationAdvection = new SimulationAdvection;
		VisSweet<SimulationAdvection> visSweet(simulationAdvection);
		delete simulationAdvection;
		return 0;
	}
#endif

	/*
	 * iterate over resolutions, starting by res[0] given e.g. by program parameter -n
	 */
	// allocate data storage for computed errors

	bool error_detected = false;

	if (parameters.bogus_var4 == 0)
	{
		std::ostringstream output_string_conv;

		double *computed_errors = new double[1024];
		double *conv_rate = new double[1024];

		std::size_t res_x = parameters.res[0];
		std::size_t res_y = parameters.res[1];

		std::size_t max_res = 128;

		if (res_x > max_res || res_y > max_res)
			max_res = std::max(res_x, res_y);

		for (	int res_iterator_id = 0;
				res_x <= max_res && res_y <= max_res;
				res_x *= 2, res_y *= 2, res_iterator_id++
		)
		{
			output_string_conv << std::endl;
			output_string_conv << res_x << "x" << res_y << "\t";

			std::cout << "*******************************************************************************" << std::endl;
			std::cout << "Testing convergence with resolution " << res_x << " x " << res_y << " and RK order " << parameters.timestepping_runge_kutta_order << std::endl;
			std::cout << "*******************************************************************************" << std::endl;

			parameters.res[0] = res_x;
			parameters.res[1] = res_y;
			parameters.reset();

			SimulationAdvection *simulationAdvection = new SimulationAdvection;

			Stopwatch time;
			time.reset();

			while(true)
			{
				if (parameters.verbosity > 0)
					std::cout << "time: " << parameters.status_simulation_time << std::endl;

				simulationAdvection->run_timestep();

				if (simulationAdvection->instability_detected())
				{
					std::cout << "INSTABILITY DETECTED" << std::endl;
					break;
				}

				bool print_output = false;
				if (turnaround_time <= parameters.status_simulation_time)
					print_output = true;

				if (parameters.max_simulation_time != -1)
					if (parameters.status_simulation_time >= parameters.max_simulation_time)
						print_output = true;

				if (print_output)
				{
					double &this_error = computed_errors[res_iterator_id];
					double &this_conv_rate_space = conv_rate[res_iterator_id];

					double error = compute_current_error(simulationAdvection);
					std::cout << "RMS error in height: " << error << std::endl;

//					double error_max = (simulationAdvection->prog_h-benchmark_h).reduce_maxAbs();
//					std::cout << "Max error in height: " << error_max << std::endl;

					this_error = error;

					double eps = 0.1;
					/*
					 * check convergence in space
					 */
					if (res_iterator_id > 0)
					{
						double &prev_error_space = computed_errors[(res_iterator_id-1)];

						double expected_conv_rate = std::pow(2.0, (double)(parameters.timestepping_runge_kutta_order));
						double this_conv_rate_space = prev_error_space / this_error;

						std::cout << "          Norm2 convergence rate (space): " << this_conv_rate_space << ", expected: " << expected_conv_rate << std::endl;

						if (std::abs(this_conv_rate_space-expected_conv_rate) > eps*expected_conv_rate)
						{
							if (error < 10e-12)
							{
								std::cerr << "Warning: Ignoring this error, since it's below machine precision" << std::endl;
							}
							else
							{
								std::cerr << "Convergence rate threshold (" << eps*expected_conv_rate << ") exceeded" << std::endl;
								error_detected = true;
							}
						}

						output_string_conv << this_conv_rate_space << "\t";
					}
					break;
				}
			}	// while true

			time.stop();

			double seconds = time();

			std::cout << "Simulation time: " << seconds << " seconds" << std::endl;
			std::cout << "Time per time step: " << seconds/(double)parameters.status_timestep_nr << " sec/ts" << std::endl;

			delete simulationAdvection;

		}	// res

		delete [] computed_errors;
		delete [] conv_rate;

		std::cout << std::endl;
		std::cout << "Convergence rate in space (inc. resolution):";
		std::cout << output_string_conv.str() << std::endl;
	}
	else if (parameters.bogus_var4 == 1)
	{
		std::ostringstream output_string_conv;

		double *computed_errors = new double[1024];
		double *conv_rate = new double[1024];

		std::size_t res_x = parameters.res[0];
		std::size_t res_y = parameters.res[1];

		double cfl_limitation = parameters.sim_CFL;

		double end_cfl = 0.0025;
		for (	int cfl_iterator_id = 0;
				cfl_iterator_id < 7;
//				cfl_limitation > end_cfl || cfl_limitation < -end_cfl;
				cfl_limitation *= 0.5, cfl_iterator_id++
		)
		{
			parameters.sim_CFL = cfl_limitation;

			output_string_conv << std::endl;
			output_string_conv << "CFL=" << parameters.sim_CFL << "\t";

			std::cout << "*********************************************************************************************************" << std::endl;
			std::cout << "Testing time convergence with CFL " << parameters.sim_CFL << " and RK order " << parameters.timestepping_runge_kutta_order << std::endl;
			std::cout << "*********************************************************************************************************" << std::endl;

			SimulationAdvection *simulationAdvection = new SimulationAdvection;

			Stopwatch time;
			time.reset();

			while(true)
			{
				if (parameters.verbosity > 0)
					std::cout << "time: " << parameters.status_simulation_time << std::endl;

				simulationAdvection->run_timestep();

				if (simulationAdvection->instability_detected())
				{
					std::cout << "INSTABILITY DETECTED" << std::endl;
					break;
				}

				bool print_output = false;
				if (turnaround_time <= parameters.status_simulation_time)
					print_output = true;

				if (parameters.max_simulation_time != -1)
					if (parameters.status_simulation_time >= parameters.max_simulation_time)
						print_output = true;

				if (print_output)
				{
					double &this_error = computed_errors[cfl_iterator_id];
					double &this_conv_rate_space = conv_rate[cfl_iterator_id];

					double error = compute_current_error(simulationAdvection);
					std::cout << "Error in height: " << error << std::endl;

//					double error_max = (simulationAdvection->prog_h-benchmark_h).reduce_maxAbs();
//					std::cout << "Max error in height: " << error_max << std::endl;

					std::cout << "          dt = " << parameters.status_simulation_timestep_size << "    dx = " << parameters.sim_cell_size[0] << " x " << parameters.sim_cell_size[0] << std::endl;

					this_error = error;

					double eps = 0.1;
					/*
					 * check convergence in time
					 */
					if (cfl_iterator_id > 0)
					{
						double &prev_error_space = computed_errors[(cfl_iterator_id-1)];

						double expected_conv_rate = std::pow(2.0, (double)(parameters.timestepping_runge_kutta_order));
						double this_conv_rate_space = prev_error_space / this_error;

						std::cout << "          Norm2 convergence rate (time): " << this_conv_rate_space << ", expected: " << expected_conv_rate << std::endl;

						if (std::abs(this_conv_rate_space-expected_conv_rate) > eps*expected_conv_rate)
						{
							if (error < 10e-12)
							{
								std::cerr << "Warning: Ignoring this error, since it's below machine precision" << std::endl;
							}
							else
							{
								std::cerr << "Convergence rate threshold (" << eps*expected_conv_rate << ") exceeded" << std::endl;
								error_detected = true;
							}
						}

						output_string_conv << "r=" << this_conv_rate_space << "\t";
						output_string_conv << "dt=" << parameters.status_simulation_timestep_size << "\t";
						output_string_conv << "dx=" << parameters.sim_cell_size[0] << "." << parameters.sim_cell_size[0];
					}
					break;
				}
			}	// while true

			time.stop();

			double seconds = time();

			std::cout << "Simulation time: " << seconds << " seconds" << std::endl;
			std::cout << "Time per time step: " << seconds/(double)parameters.status_timestep_nr << " sec/ts" << std::endl;

			delete simulationAdvection;

		}	// res

		delete [] computed_errors;
		delete [] conv_rate;

		std::cout << std::endl;
		std::cout << "Convergence rate in time (inc. resolution):";
		std::cout << output_string_conv.str() << std::endl;
	}
	else
	{
		std::cout << "Use -e [0/1] to specify convergence test: 0 = spatial refinement, 1 = time refinement" << std::endl;
	}

	if (error_detected)
	{
		std::cerr << "There was an error in the convergence tests" << std::endl;
		exit(1);
	}

	return 0;
}