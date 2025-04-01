# Copyright 2014-2017 The MathWorks, Inc.
#
# File    : macsdkver.pl
# Abstract:
#       Determine the SDK version to use for the MAC OS
#

# Capture both stdout and stderr from xcodebuild. This means both
# stdout and stderr will be parsed (instead of passing any output in
# in stderr directly to the caller)
my @sdks = `xcodebuild -showsdks 2>&1`;

# Use the lowest macosx version 'xcodebuild -showsdks' returns.
my $sdkver = undef;
my @verlist = ();
foreach my $line (@sdks){
    if($line =~ /\-sdk macosx([\d\.]+)/){
        my @aver = split('\.',$1);
        push(@verlist,$1);
    }
}

if(scalar(@verlist)>0){
    @verlist = sort(@verlist);
    print $verlist[0];
}else{
    die "Please download a supported version of Xcode";
}

#[eof] macsdkver.pl
