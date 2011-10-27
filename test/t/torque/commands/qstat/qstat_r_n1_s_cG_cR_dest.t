#!/usr//bin/perl

use strict;
use warnings;

use FindBin;
use TestLibFinder;
use lib test_lib_loc();


# Test Modules
use CRI::Test;


use Torque::Job::Ctrl          qw(
                                   submitSleepJob
                                   runJobs
                                   delJobs 
                                 );
use Torque::Test::Regexp       qw(
                                   QSTAT_A_REGEXP
                                 );
use Torque::Util        qw(
                                   run_and_check_cmd
                                   job_info
                                 );
use Torque::Test::Qstat::Utils qw(
                                   parse_qstat_a_e_i_r
                                 );

# Test Description
plan('no_plan');
setDesc("qstat -r -n -1 -s -G -R [destination]");

# Variables
my $cmd;
my %qstat;
my @job_ids;
my %job_info;
my $job_params;

my $queue      = $props->get_property( 'torque.queue.one'   );
my $user       = $props->get_property( 'User.1'    );
my $torque_bin = $props->get_property( 'Torque.Home.Dir' ) . '/bin/';

my @attributes = qw(
                     job_id
                     username
                     queue
                     nds
                     tsk
                     req_mem
                     req_time
                     state
                     elap_time
                     big
                     fast
                     pfs
                     comment
                   );

# Submit a job
$job_params = {
                'user'       => $user,
                'torque_bin' => $torque_bin,
                'args'       => "-q $queue -l mem=1024mb"
              };

# submit a couple of jobs
push(@job_ids, submitSleepJob($job_params));
push(@job_ids, submitSleepJob($job_params));

# Run Jobs
runJobs(@job_ids);

# Test qstat
$cmd   = "qstat -r -n -1 -s -G -R $queue";
%qstat = run_and_check_cmd($cmd);

%job_info = parse_qstat_a_e_i_r( $qstat{ 'STDOUT' } );

foreach my $job_id (@job_ids)
  {

  my $msg = "Checking job '$job_id'";
  diag($msg);
  logMsg($msg);

  foreach my $attribute (@attributes)
    {

    my $reg_exp = &QSTAT_A_REGEXP->{ $attribute };
    like($job_info{ $job_id }{ $attribute }, $reg_exp, "Checking for '$job_id' $attribute attribute"); 

    } # END foreach my $attribute (@attribues)

  } # END foreach my $job_id (@job_ids)

# Delete the job
delJobs(@job_ids);