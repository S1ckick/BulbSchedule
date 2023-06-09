#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/algos.o \
	${OBJECTDIR}/src/bysolver.o \
	${OBJECTDIR}/src/greedy.o \
	${OBJECTDIR}/src/main.o \
	${OBJECTDIR}/src/parser.o \
	${OBJECTDIR}/src/validation.o \
	${OBJECTDIR}/src/writer.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=--std=c++17
CXXFLAGS=--std=c++17

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=third-party/or-tools-linux/lib/libCbc.a third-party/or-tools-linux/lib/libCbcSolver.a third-party/or-tools-linux/lib/libCgl.a third-party/or-tools-linux/lib/libClp.a third-party/or-tools-linux/lib/libClpSolver.a third-party/or-tools-linux/lib/libCoinUtils.a third-party/or-tools-linux/lib/libOsi.a third-party/or-tools-linux/lib/libOsiCbc.a third-party/or-tools-linux/lib/libOsiClp.a third-party/or-tools-linux/lib/libabsl_bad_any_cast_impl.a third-party/or-tools-linux/lib/libabsl_bad_optional_access.a third-party/or-tools-linux/lib/libabsl_bad_variant_access.a third-party/or-tools-linux/lib/libabsl_base.a third-party/or-tools-linux/lib/libabsl_city.a third-party/or-tools-linux/lib/libabsl_civil_time.a third-party/or-tools-linux/lib/libabsl_cord.a third-party/or-tools-linux/lib/libabsl_cord_internal.a third-party/or-tools-linux/lib/libabsl_cordz_functions.a third-party/or-tools-linux/lib/libabsl_cordz_handle.a third-party/or-tools-linux/lib/libabsl_cordz_info.a third-party/or-tools-linux/lib/libabsl_cordz_sample_token.a third-party/or-tools-linux/lib/libabsl_crc32c.a third-party/or-tools-linux/lib/libabsl_crc_cord_state.a third-party/or-tools-linux/lib/libabsl_crc_cpu_detect.a third-party/or-tools-linux/lib/libabsl_crc_internal.a third-party/or-tools-linux/lib/libabsl_debugging_internal.a third-party/or-tools-linux/lib/libabsl_demangle_internal.a third-party/or-tools-linux/lib/libabsl_die_if_null.a third-party/or-tools-linux/lib/libabsl_examine_stack.a third-party/or-tools-linux/lib/libabsl_exponential_biased.a third-party/or-tools-linux/lib/libabsl_failure_signal_handler.a third-party/or-tools-linux/lib/libabsl_flags.a third-party/or-tools-linux/lib/libabsl_flags_commandlineflag.a third-party/or-tools-linux/lib/libabsl_flags_commandlineflag_internal.a third-party/or-tools-linux/lib/libabsl_flags_config.a third-party/or-tools-linux/lib/libabsl_flags_internal.a third-party/or-tools-linux/lib/libabsl_flags_marshalling.a third-party/or-tools-linux/lib/libabsl_flags_parse.a third-party/or-tools-linux/lib/libabsl_flags_private_handle_accessor.a third-party/or-tools-linux/lib/libabsl_flags_program_name.a third-party/or-tools-linux/lib/libabsl_flags_reflection.a third-party/or-tools-linux/lib/libabsl_flags_usage.a third-party/or-tools-linux/lib/libabsl_flags_usage_internal.a third-party/or-tools-linux/lib/libabsl_graphcycles_internal.a third-party/or-tools-linux/lib/libabsl_hash.a third-party/or-tools-linux/lib/libabsl_hashtablez_sampler.a third-party/or-tools-linux/lib/libabsl_int128.a third-party/or-tools-linux/lib/libabsl_leak_check.a third-party/or-tools-linux/lib/libabsl_log_entry.a third-party/or-tools-linux/lib/libabsl_log_flags.a third-party/or-tools-linux/lib/libabsl_log_globals.a third-party/or-tools-linux/lib/libabsl_log_initialize.a third-party/or-tools-linux/lib/libabsl_log_internal_check_op.a third-party/or-tools-linux/lib/libabsl_log_internal_conditions.a third-party/or-tools-linux/lib/libabsl_log_internal_format.a third-party/or-tools-linux/lib/libabsl_log_internal_globals.a third-party/or-tools-linux/lib/libabsl_log_internal_log_sink_set.a third-party/or-tools-linux/lib/libabsl_log_internal_message.a third-party/or-tools-linux/lib/libabsl_log_internal_nullguard.a third-party/or-tools-linux/lib/libabsl_log_internal_proto.a third-party/or-tools-linux/lib/libabsl_log_severity.a third-party/or-tools-linux/lib/libabsl_log_sink.a third-party/or-tools-linux/lib/libabsl_low_level_hash.a third-party/or-tools-linux/lib/libabsl_malloc_internal.a third-party/or-tools-linux/lib/libabsl_periodic_sampler.a third-party/or-tools-linux/lib/libabsl_random_distributions.a third-party/or-tools-linux/lib/libabsl_random_internal_distribution_test_util.a third-party/or-tools-linux/lib/libabsl_random_internal_platform.a third-party/or-tools-linux/lib/libabsl_random_internal_pool_urbg.a third-party/or-tools-linux/lib/libabsl_random_internal_randen.a third-party/or-tools-linux/lib/libabsl_random_internal_randen_hwaes.a third-party/or-tools-linux/lib/libabsl_random_internal_randen_hwaes_impl.a third-party/or-tools-linux/lib/libabsl_random_internal_randen_slow.a third-party/or-tools-linux/lib/libabsl_random_internal_seed_material.a third-party/or-tools-linux/lib/libabsl_random_seed_gen_exception.a third-party/or-tools-linux/lib/libabsl_random_seed_sequences.a third-party/or-tools-linux/lib/libabsl_raw_hash_set.a third-party/or-tools-linux/lib/libabsl_raw_logging_internal.a third-party/or-tools-linux/lib/libabsl_scoped_set_env.a third-party/or-tools-linux/lib/libabsl_spinlock_wait.a third-party/or-tools-linux/lib/libabsl_stacktrace.a third-party/or-tools-linux/lib/libabsl_status.a third-party/or-tools-linux/lib/libabsl_statusor.a third-party/or-tools-linux/lib/libabsl_str_format_internal.a third-party/or-tools-linux/lib/libabsl_strerror.a third-party/or-tools-linux/lib/libabsl_strings.a third-party/or-tools-linux/lib/libabsl_strings_internal.a third-party/or-tools-linux/lib/libabsl_symbolize.a third-party/or-tools-linux/lib/libabsl_synchronization.a third-party/or-tools-linux/lib/libabsl_throw_delegate.a third-party/or-tools-linux/lib/libabsl_time.a third-party/or-tools-linux/lib/libabsl_time_zone.a third-party/or-tools-linux/lib/libprotobuf-lite.a third-party/or-tools-linux/lib/libprotobuf.a third-party/or-tools-linux/lib/libprotoc.a third-party/or-tools-linux/lib/libre2.a third-party/or-tools-linux/lib/libscip.a third-party/or-tools-linux/lib/libz.a third-party/or-tools-linux/lib/libortools.so third-party/or-tools-linux/lib/libortools_flatzinc.so

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libCbc.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libCbcSolver.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libCgl.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libClp.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libClpSolver.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libCoinUtils.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libOsi.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libOsiCbc.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libOsiClp.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_bad_any_cast_impl.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_bad_optional_access.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_bad_variant_access.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_base.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_city.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_civil_time.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_cord.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_cord_internal.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_cordz_functions.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_cordz_handle.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_cordz_info.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_cordz_sample_token.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_crc32c.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_crc_cord_state.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_crc_cpu_detect.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_crc_internal.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_debugging_internal.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_demangle_internal.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_die_if_null.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_examine_stack.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_exponential_biased.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_failure_signal_handler.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_flags.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_flags_commandlineflag.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_flags_commandlineflag_internal.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_flags_config.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_flags_internal.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_flags_marshalling.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_flags_parse.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_flags_private_handle_accessor.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_flags_program_name.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_flags_reflection.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_flags_usage.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_flags_usage_internal.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_graphcycles_internal.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_hash.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_hashtablez_sampler.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_int128.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_leak_check.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_log_entry.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_log_flags.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_log_globals.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_log_initialize.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_log_internal_check_op.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_log_internal_conditions.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_log_internal_format.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_log_internal_globals.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_log_internal_log_sink_set.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_log_internal_message.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_log_internal_nullguard.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_log_internal_proto.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_log_severity.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_log_sink.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_low_level_hash.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_malloc_internal.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_periodic_sampler.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_random_distributions.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_random_internal_distribution_test_util.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_random_internal_platform.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_random_internal_pool_urbg.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_random_internal_randen.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_random_internal_randen_hwaes.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_random_internal_randen_hwaes_impl.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_random_internal_randen_slow.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_random_internal_seed_material.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_random_seed_gen_exception.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_random_seed_sequences.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_raw_hash_set.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_raw_logging_internal.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_scoped_set_env.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_spinlock_wait.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_stacktrace.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_status.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_statusor.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_str_format_internal.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_strerror.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_strings.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_strings_internal.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_symbolize.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_synchronization.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_throw_delegate.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_time.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libabsl_time_zone.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libprotobuf-lite.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libprotobuf.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libprotoc.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libre2.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libscip.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libz.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libortools.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: third-party/or-tools-linux/lib/libortools_flatzinc.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/src/algos.o: src/algos.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Isrc -Ithird-party -Isrc/algos -I../or-tools/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/algos.o src/algos.cpp

${OBJECTDIR}/src/bysolver.o: src/bysolver.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Isrc -Ithird-party -Isrc/algos -I../or-tools/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/bysolver.o src/bysolver.cpp

${OBJECTDIR}/src/greedy.o: src/greedy.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Isrc -Ithird-party -Isrc/algos -I../or-tools/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/greedy.o src/greedy.cpp

${OBJECTDIR}/src/main.o: src/main.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Isrc -Ithird-party -Isrc/algos -I../or-tools/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/main.o src/main.cpp

${OBJECTDIR}/src/parser.o: src/parser.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Isrc -Ithird-party -Isrc/algos -I../or-tools/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/parser.o src/parser.cpp

${OBJECTDIR}/src/validation.o: src/validation.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Isrc -Ithird-party -Isrc/algos -I../or-tools/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/validation.o src/validation.cpp

${OBJECTDIR}/src/writer.o: src/writer.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Isrc -Ithird-party -Isrc/algos -I../or-tools/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/writer.o src/writer.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} -r ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libortools.so ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libortools_flatzinc.so
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/bulbschedule

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
