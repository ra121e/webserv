#!/usr/bin/env perl
use strict;
use warnings;
use utf8;
binmode STDOUT, ":utf8";

# Build a library of 1000 inspirational quotes procedurally from safe fragments.
my @openings = (
  'Believe in','Focus on','Keep','Embrace','Trust','Pursue','Build','Create','Dream of','Act with',
  'Practice','Choose','Cultivate','Nurture','Spark','Find','Honor','Elevate','Protect','Celebrate'
);
my @nouns = (
  'your vision','progress','small steps','the journey','momentum','resilience','your craft','your strengths',
  'new beginnings','opportunity','possibility','your purpose','discipline','your values','growth','curiosity',
  'your potential','consistency','kindness','courage','clarity','focus','patience','gratitude','learning'
);
my @adverbs = (
  'boldly','daily','deeply','fearlessly','patiently','relentlessly','with gratitude','with purpose',
  'with courage','with clarity','with kindness','with curiosity','with heart','with humility','with joy'
);
my @endings = (
  'and the rest will follow.','even when no one is watching.','one step at a time.','especially when it’s hard.',
  'and keep moving forward.','because you are capable.','and make it happen.','and enjoy the process.',
  'and write your story.','and trust the timing.','and lead with heart.','and let it compound.',
  'and let your actions speak.','and keep showing up.','and start where you are.'
);

my %seen;
my @quotes;
for my $o (@openings) {
  last if @quotes >= 1000;
  for my $n (@nouns) {
    last if @quotes >= 1000;
    for my $a (@adverbs) {
      last if @quotes >= 1000;
      for my $e (@endings) {
        my $q = "$o $n, $a $e";
        next if $seen{$q}++;
        push @quotes, $q;
        last if @quotes >= 1000;
      }
    }
  }
}

# Fallback in case combinations are fewer (shouldn’t happen with sets above)
if (@quotes < 1000) {
  my $base = scalar @quotes;
  for (my $i = 0; $i < 1000 - $base; $i++) {
    push @quotes, $quotes[$i % $base];
  }
}

srand();
my $idx = int(rand(scalar @quotes));
my $quote = $quotes[$idx];

print "HTTP/1.1 200 OK\r\n";
print "Content-Type: text/plain; charset=utf-8\r\n";
print "Cache-Control: no-store\r\n";
print "\r\n";
print $quote, "\n";
