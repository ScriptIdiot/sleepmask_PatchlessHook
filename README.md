# sleepmask_PatchlessHook
Code snippets to add on top of cobalt strike sleepmask kit so that patchless hook on AMSI and ETW can be achieved.

_Only for experimental purpose._
_Always test to make sure its working as intended_

From my peronsal view, suggest not to use it in existing process (i.e. inject), but use it in a newly spawned process (i.e. spawn)

## Feature
1. Breakpoint will be removed during sleep to avoid scanner (I hope lol)
2. Avoid scanner like moneta that will detect DLL has been modified.

## Screenshots
![image](https://user-images.githubusercontent.com/21979646/225378093-d6de7925-ee56-4bc5-b17d-40836767bafd.png)
![image](https://user-images.githubusercontent.com/21979646/225378129-22a3ed57-f024-4bab-807b-2830bda11986.png)

## Usage
1. Include "patchless.c" in sleepmask.c (only supports x64)
2. Add the functions required to do patchless hook on desired functions
- You may refer to sleepmask.c to see what have been amended
3. Put patchless.c in src47 folder
4. Compile

## Caveat
1. It cannot cater if your action will create new thread during the execution period of time, which means newly spawned threads at that specific period will not have patchless hook. Theoretically, the newly spawned thread(s) will have patchless hook after one sleep cycle.
2. If you want to address above caveat, you may hook NtCreateThreadEx to do so.
3. At the moment, it only caters x64.

## Credits
All credits to [@rad9800](https://github.com/rad9800)
