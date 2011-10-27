#!/usr/bin/perl
use strict;
use warnings;

use FindBin;
use TestLibFinder;
use lib test_lib_loc();

use CRI::Test;
use Torque::Ctrl;
use Torque::Util::Pbsnodes qw( pbsnodes );
use Torque::Job::Ctrl   qw(
                            submitSleepJob
                            delJobs
                          );
use CRI::Util qw(
                            is_running
                            );
use Torque::Util qw(
                            verify_job_state
                          );
plan('no_plan');
setDesc('pbs_sched');

my @job_ids;

my $pbs_sched_cmd      = "pbs_sched";

my $job_state_params;

my %pbs_sched;

###############################################################################
# Setup for the test
###############################################################################
stopTorque();
stopPbssched();
startTorqueClean();

my %nref = pbsnodes();
my $np   = (values %nref)[0]->{np};

###############################################################################
# Test the pbs_sched command
###############################################################################
%pbs_sched = runCommand($pbs_sched_cmd);
ok($pbs_sched{ 'EXIT_CODE' } == 0, "Checking exit code of '$pbs_sched_cmd'")
  or diag("EXIT_CODE: $pbs_sched{ 'EXIT_CODE' }\nSTDERR: $pbs_sched{ 'STDERR' }");

# Wait for pbs_sched to stabilize
diag("Waiting for pbs_sched to stabilize...");
sleep_diag 5;

# Make sure that pbs_sched has started
ok(is_running('pbs_sched'), "Verifying that pbs_sched is running");

###############################################################################
# Test pbs_sched's scheduling
###############################################################################
my $job_params         = { add_args => "-l nodes=1:ppn=$np" };
push(@job_ids, submitSleepJob($job_params));
push(@job_ids, submitSleepJob($job_params));
push(@job_ids, submitSleepJob($job_params));

# Check job 0
$job_state_params = {
                      'job_id'        => $job_ids[0],
                      'exp_job_state' => 'R',
                      'wait_time'     => 30
                    };
verify_job_state($job_state_params);

# Check job 1
$job_state_params = {
                      'job_id'        => $job_ids[1],
                      'exp_job_state' => 'Q',
                      'wait_time'     => 30
                    };
verify_job_state($job_state_params);

# Check job 2
$job_state_params = {
                      'job_id'        => $job_ids[2],
                      'exp_job_state' => 'Q',
                      'wait_time'     => 30
                    };
verify_job_state($job_state_params);

###############################################################################
# Cleanup after the test
###############################################################################
stopPbssched();