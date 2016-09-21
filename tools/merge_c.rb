#!/usr/bin/env ruby

PATHS = %w(. out libc)

def merge(c, used)
  c.gsub(/#\s*include\s*[<"](.*?)[>"]/) do
    name = $1
    fn = PATHS.map{|p|"#{p}/#{name}"}.find{|n|File.exist?(n)}
    if !fn
      raise "#include not found: #{name}"
    end
    if used[fn]
      ''
    else
      used[fn] = true
      nc = File.read(fn)
      merge(nc, used)
    end
  end
end

c = File.read(ARGV[0])
puts merge(c, {})
