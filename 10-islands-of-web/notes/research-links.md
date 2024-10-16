# Research

Nowhere else but in web development can things get so messy so fast that it is
relatable that web developers keep looking for that magical framework that will
bring sanity back into their lives, including mine.

The reality however is that there is no such magical framework; loads of them
hold promise and honestly if you stay within the guard rails you can go quite
the distance. The trouble is when you have a use case that cannot stay within.

At times like those, it is best to have picked a framework that is loose on
policy and rich on mechanisms; note that not having a mechanism qualifies
because you can build it yourself. Provided you know how to navigate.

Policy rich frameworks such as AstroJS, SvelteKit and NextJS are great and get
the job done at most job places. But I can't shake that feeling that there will
come a use case which I won't be able to address because the framework won't
solve it and I have no clue how to go deep into the framework to get it resolved
myself.

So that's my excuse to maintain this list of alternatives that claim to be as
barebones as they can be and yet provide 80% of what their bigger cousins offer.

## [Iles, The Joyful Site Generator](https://iles.pages.dev/)

Seems similar to AstroJS the most; Vue is the lingua franca:

- Partial Hydration (+)
- Multiframework (+)
- Markdown Support (+)
- Batteries Included (?)
- File-based routing (+)

Source:

- https://github.com/ElMassimo/iles
- Development activity seems to be on the slow track

## [Tropical, Real HTML with islands](https://tropical.js.org/)

Refreshingly this is template repo with some opinionated Vite configuration.
This is more to my liking. Only for React though. I would like to someday mess
around with it to support Svelte 5. I don't like however the use of `fela` since
I am not a fan of `css in js`. See below: it is just yucky.

```js
import React from "react";
import { useFela } from "react-fela";

function Button({ children, fontSize = 16 }) {
  const { css } = useFela();

  return (
    <button
      className={css({
        padding: 10,
        fontSize,
        color: "darkblue",
        ":hover": {
          color: "blue",
        },
      })}
    >
      {children}
    </button>
  );
}
```

Source:

- https://github.com/bensmithett/tropical
- https://github.com/bensmithett/tropical-utils
