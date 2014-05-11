#!/usr/bin/perl

use strict;
use warnings;

use XML::Simple;
use DBI;

if (@ARGV && $ARGV[0] =~ /^(?:--help|-h)$/) {
    print "Usage: $$ [xml_file_name] [db_f_name]\n";
    exit 0;
}

my $fname = "haarcascade_frontalface_alt.xml";
$fname = shift @ARGV if @ARGV;

my $db_f_name = "assets/faces.db3";
$db_f_name = shift @ARGV if @ARGV;

my $dbh = DBI->connect("dbi:SQLite:$db_f_name") or die "Can't connect to database: $DBI::errstr\n";
my %requests = (
    add_feature => $dbh->prepare("insert into features(tilted) values (?)"),
    add_stage   => $dbh->prepare("insert into stages(threshold, parent_stage, next_stage) values (?, ?, ?)"),
    add_node    => $dbh->prepare("insert into nodes(feature_id, stage_id, threshold, left_val, right_val) values (?, ?, ?, ?, ?)"),
    add_rect    => $dbh->prepare("insert into rects(feature_id, x, y, width, height, weight) values (?, ?, ?, ?, ?, ?)"),
    get_id      => $dbh->prepare("select last_insert_rowid()"),
);

my $data = XMLin($fname);
$data = $data->{haarcascade_frontalface_alt}->{stages}->{_};

$dbh->do("begin transaction");

for my $stage (@$data) {
    my $parent = $stage->{parent};
    my $next = $stage->{next};

    $parent = $data->[$parent]->{stage_id} if $parent != -1;
    $next = $data->[$next]->{stage_id} if $next != -1;

    $requests{add_stage}->execute($stage->{stage_threshold}, $parent, $next);

    my $sth = $requests{get_id};
    $sth->execute;
    $stage->{stage_id} = $sth->fetchrow_arrayref()->[0];

    for my $tree (@{$stage->{trees}->{_}}) {
        $tree = $tree->{_};
        my $feature = $tree->{feature};
        $requests{add_feature}->execute($feature->{tilted});

        $sth->execute;
        $feature->{feature_id} = $sth->fetchrow_arrayref()->[0];

        for my $rect (@{$feature->{rects}->{_}}) {
            $requests{add_rect}->execute($feature->{feature_id}, split(/ /, $rect));
        }

        $requests{add_node}->execute($feature->{feature_id}, $stage->{stage_id}, $tree->{threshold}, $tree->{left_val}, $tree->{right_val});
    }
}

$dbh->do("commit");
