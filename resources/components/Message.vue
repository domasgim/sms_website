<template>
  <div class="container-table-message">
    <div class="info">
      <p class="title">
        {{ numberTitle }}
      </p>
      <p class="text" v-if="numberTitle === 'Sender'">
        {{ message.sender }}
      </p>
      <p class="text" v-if="numberTitle === 'Recipient'">
        {{ message.recipient }}
      </p>
    </div>
    <div class="content" ref="content">
      <div class="row">
        <p class="title">
          Message
        </p>
        <img v-if="numberTitle === 'Sender'" src="/images/eye.svg" @click="markRead" class="eye" :class="{ 'active' : message.read == false }">
      </div>
      <p class="text">
        {{ message.content }}
      </p>
      <img src="/images/triangle-empty.svg" class="arrow" ref="arrow" @click="noWrap" v-if="overflown">
    </div>
    <div class="info">
      <p class="title">
        {{ dateTitle }}
      </p>
      <p class="text">
        {{ message.created_at }}
      </p>
    </div>
  </div>
</template>

<script>
export default {
  name: "Message",
  data() {
    return {
      overflown: false
    };
  },
  props: {
    item: Object,
    numberTitle: String,
    dateTitle: String
  },
  computed: {
    message () {
      return this.item
    },
  },
  methods: {
    noWrap () {
      if (this.overflown) {
        this.$refs.content.parentNode.classList.toggle('height-auto')
        this.$refs.arrow.classList.toggle('rotate-arrow')
      }
    },
    markRead () {
      this.$store.commit('updateReadStatus', {
          id: this.message.id,
          read: !this.message.read
        })
    }
  },
  mounted () {
    this.$nextTick(() => {
      const element = this.$refs.content
      const overflow = (element.offsetHeight < element.scrollHeight || element.offsetWidth < element.scrollWidth)
      this.overflown = overflow
    })
  }
};
</script>

<style lang="scss" scoped>
$border: #e9e9e9;
.container-table-message {
  display: flex;
  flex-direction: row;
  flex-wrap: nowrap;
  font-size: 1em;
  line-height: 1.5em;
  width: 100%;
  height: 150px;
  margin-bottom: 2em;
  border: 1px solid $border;
  border-radius: 20px;
  box-shadow: inset 0px 0px 6px rgba(0, 0, 0, 0.2);
  :nth-child(3) {
    border-left: 2px solid $border;
    border-right: none;
  }
}
.info {
  flex-basis: 20%;
  font-size: 1em;
  text-align: center;
  border-right: 2px solid $border;
}
.content {
  position: relative;
  flex-basis: 60%;
  color: #4F4F4F;
  overflow: hidden;
  .text {
    padding: 5px 1.5em 5px 0.5em;
    word-wrap: break-word;
    hyphens: auto;
  }
  .arrow {
    position: absolute;
    bottom: 0;
    right: 0;
    padding: 3px 0;
    width: 22px;
    height: 22px;
    line-height: 20px;
    text-align: center;
    font-size: 1.5em;
    font-weight: 900;
    color: #0054a6;
    border: 0;
    border-right: 2px solid $border;
    border-bottom: 2px solid $border;
    border-bottom-right-radius: 10px;
    user-select: none;
    transform: rotate(180deg);
    cursor: pointer;
    &:hover {
      background-color: rgba(0, 0, 0, 0.05);
    }
  }
}

.eye {
  height: 40px;
  padding: 0.5em;
  opacity: 0.4;
  cursor: pointer;
}
.row {
  display: flex;
  flex-direction: row;
  border-bottom: 2px solid $border;
  justify-content: space-between;
  .title {
    border: none;
  }
  .active {
    filter: invert(18%) sepia(99%) saturate(2104%) hue-rotate(196deg) brightness(97%) contrast(101%);
    opacity: 1;
  }
}
.title {
  border-bottom: 2px solid $border;
  padding: 0.5em;
}
@media only screen and (max-width: 780px) {
  .container-table-message {
    display: flex;
    flex-direction: column;
    min-height: 202px;
    > :nth-child(3) {
      border-top: 2px solid $border;
      border-left: none;
      border-bottom: none;
    }
  }
  .info {
    display: flex;
    flex-direction: row;
    border-right: none;
    flex-basis: auto;
    justify-content: space-between;
    //border-bottom: 2px solid $border;
    .title {
      flex-basis: 49%;
      text-align: right;
      border: none;
    }
    .text {
      flex-basis: 49%;
      text-align: left;
      padding: 0.5em;
    }
  }
  .content {
    .title {
      display: none;
    }
    .text {
      line-height: 27px;
    }
  }
  .eye {
    margin-left: auto;
    margin-right: auto;
  }
}
.height-auto {
  height: auto;
}
.rotate-arrow {
  transform: rotate(0) !important;
  border: 0 !important;
  border-radius: 0 !important;
  border-top-left-radius: 10px !important;
  border-left: 2px solid $border !important;
  border-top: 2px solid $border !important;
}
</style>
