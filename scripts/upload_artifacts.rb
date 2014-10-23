#!/usr/bin/env ruby

begin
  gem 'fog'
rescue LoadError
  system('gem install --no-rdoc --no-ri fog')
  Gem.clear_paths
end

require 'fileutils'
require 'fog'

def revision
  ENV.fetch('REVISION')[0...8]
end

def branch
  ENV.fetch('BRANCH_NAME').gsub(/[\.\/]+/, '_')
end

def build_number
  ENV.fetch('SEMAPHORE_BUILD_NUMBER')
end

def project_dir
  ENV.fetch('SEMAPHORE_PROJECT_DIR')
end

def steam_dir
  File.join(ENV.fetch('SEMAPHORE_CACHE_DIR'), '.steamcmd')
end

def tarball_name
  "#{branch}-#{build_number}-#{revision}.tar.gz"
end

def tarball_location
  File.join(project_dir, tarball_name)
end

def artifacts
  [
    File.join(project_dir, 'premake/gmake/build'),
    File.join(project_dir, 'srcds.log'),
    File.join(steam_dir, 'orangebox/debug.log')
  ].select{|f| File.exist?(f)}
end

bucket = Fog::Storage.new(
  provider: 'AWS',
  aws_access_key_id: ENV.fetch('AWS_ACCESS_KEY_ID'),
  aws_secret_access_key: ENV.fetch('AWS_SECRET_ACCESS_KEY'),
).directories.get('semaphore-sizzlingstats')

FileUtils.mkdir_p File.join(project_dir, 'artifacts')
FileUtils.cp_r artifacts, File.join(project_dir, 'artifacts')

`cd #{project_dir} && tar cvfz #{tarball_location} artifacts`

puts "Uploading build artifacts to S3"
file = bucket.files.create(
  key: tarball_name,
  body: File.open(tarball_location)
)

puts "Uploaded artifacts to https://s3.amazonaws.com/semaphore-sizzlingstats/#{tarball_name}"
