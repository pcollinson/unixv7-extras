# Missing [ command

I found that on my distribution the __[__ command was missing. This should be a link to the _test_ command and makes the shell syntax for __if__ work.

``` sh
if [ -f /bin/test]; then
	echo 'test exists'
```

The script here checks for the existence of ``[`` and makes a link if needed.
